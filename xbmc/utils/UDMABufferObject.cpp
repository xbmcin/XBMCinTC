/*
 *  Copyright (C) 2005-2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "UDMABufferObject.h"

#include "utils/BufferObjectFactory.h"
#include "utils/log.h"

#include <drm/drm_fourcc.h>
#include <linux/udmabuf.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

namespace
{

const auto PAGESIZE = getpagesize();

int RoundUp(int num, int factor)
{
  return num + factor - 1 - (num - 1) % factor;
}

} // namespace

std::unique_ptr<CBufferObject> CUDMABufferObject::Create()
{
  return std::make_unique<CUDMABufferObject>();
}

void CUDMABufferObject::Register()
{
  int fd = open("/dev/udmabuf", O_RDWR);
  if (fd < 0)
  {
    CLog::LogF(LOGERROR, "unable to open /dev/udmabuf (check your permissions)");
    return;
  }

  close(fd);

  CBufferObjectFactory::RegisterBufferObject(CUDMABufferObject::Create);
}

CUDMABufferObject::~CUDMABufferObject()
{
  ReleaseMemory();
  DestroyBufferObject();

  int ret = close(m_udmafd);
  if (ret < 0)
    CLog::LogF(LOGERROR, "close /dev/udmabuf failed, errno={}", strerror(errno));

  m_udmafd = -1;
}

bool CUDMABufferObject::CreateBufferObject(uint32_t format, uint32_t width, uint32_t height)
{
  if (m_fd >= 0)
    return true;

  uint32_t bpp{1};

  switch (format)
  {
    case DRM_FORMAT_ARGB8888:
      bpp = 4;
      break;
    case DRM_FORMAT_ARGB1555:
    case DRM_FORMAT_RGB565:
      bpp = 2;
      break;
    default:
      throw std::system_error(errno, std::generic_category(), "pixel format not implemented");
  }

  // Must be rounded to the system page size
  m_size = RoundUp(width * height * bpp, PAGESIZE);
  m_stride = width * bpp;

  m_memfd = memfd_create("kodi", MFD_CLOEXEC | MFD_ALLOW_SEALING);
  if (m_memfd < 0)
  {
    CLog::LogF(LOGERROR, "memfd_create failed: {}", strerror(errno));
    return false;
  }

  if (ftruncate(m_memfd, m_size) < 0)
  {
    CLog::LogF(LOGERROR, "ftruncate failed: {}", strerror(errno));
    return false;
  }

  if (fcntl(m_memfd, F_ADD_SEALS, F_SEAL_SHRINK) < 0)
  {
    CLog::LogF(LOGERROR, "fcntl failed: {}", strerror(errno));
    close(m_memfd);
    return false;
  }

  if (m_udmafd < 0)
  {
    m_udmafd = open("/dev/udmabuf", O_RDWR);
    if (m_udmafd < 0)
    {
      CLog::LogF(LOGERROR, "unable to open /dev/udmabuf: {}", strerror(errno));
      close(m_memfd);
      return false;
    }
  }

  struct udmabuf_create_item create = {
      .memfd = static_cast<uint32_t>(m_memfd),
      .offset = 0,
      .size = m_size,
  };

  m_fd = ioctl(m_udmafd, UDMABUF_CREATE, &create);
  if (m_fd < 0)
  {
    CLog::LogF(LOGERROR, "ioctl UDMABUF_CREATE failed: {}", strerror(errno));
    close(m_memfd);
    return false;
  }

  return true;
}

void CUDMABufferObject::DestroyBufferObject()
{
  if (m_fd < 0)
    return;

  int ret = close(m_fd);
  if (ret < 0)
    CLog::LogF(LOGERROR, "close fd failed, errno={}", strerror(errno));

  ret = close(m_memfd);
  if (ret < 0)
    CLog::LogF(LOGERROR, "close memfd failed, errno={}", strerror(errno));

  m_memfd = -1;
  m_fd = -1;
  m_stride = 0;
  m_size = 0;
}

uint8_t* CUDMABufferObject::GetMemory()
{
  if (m_fd < 0)
    return nullptr;

  if (m_map)
  {
    CLog::LogF(LOGDEBUG, "already mapped fd={} map={}", m_fd, fmt::ptr(m_map));
    return m_map;
  }

  m_map = static_cast<uint8_t*>(mmap(nullptr, m_size, PROT_WRITE, MAP_SHARED, m_memfd, 0));
  if (m_map == MAP_FAILED)
  {
    CLog::LogF(LOGERROR, "mmap failed, errno={}", strerror(errno));
    return nullptr;
  }

  return m_map;
}

void CUDMABufferObject::ReleaseMemory()
{
  if (!m_map)
    return;

  int ret = munmap(m_map, m_size);
  if (ret < 0)
    CLog::LogF(LOGERROR, "munmap failed, errno={}", strerror(errno));

  m_map = nullptr;
}
