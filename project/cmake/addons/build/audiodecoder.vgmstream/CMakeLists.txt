project(audiodecoder.vgmstream)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${p8-platform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/lib/vgmstream)

add_subdirectory(lib/vgmstream)

set(VGM_SOURCES src/VGMCodec.cpp)

set(DEPLIBS ${p8-platform_LIBRARIES} vgmstream)

build_addon(audiodecoder.vgmstream VGM DEPLIBS)

include(CPack)
