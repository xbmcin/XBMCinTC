project(resid)

enable_language(CXX)

cmake_minimum_required(VERSION 2.6)

set(RESID_INLINE inline)
set(RESID_INLINING 1)
set(RESID_HAVE_BOOL 1)
set(RESID_NAMESPACE)
configure_file(${PROJECT_SOURCE_DIR}/siddefs.h.in siddefs.h)

include_directories(${CMAKE_BINARY_DIR})

add_definitions(-DVERSION="0.16" -D_CRT_SECURE_NO_WARNINGS -DHAVE_MSWINDOWS)

set(SOURCES sid.cc
            voice.cc
            wave.cc
            envelope.cc
            filter.cc
            extfilt.cc
            pot.cc
            version.cc
            wave6581_PST.cc
            wave6581_PS_.cc
            wave6581_P_T.cc
            wave6581__ST.cc
            wave8580_PST.cc
            wave8580_PS_.cc
            wave8580_P_T.cc
            wave8580__ST.cc)

add_library(resid STATIC ${SOURCES})

set(HEADERS ${PROJECT_SOURCE_DIR}/envelope.h
            ${PROJECT_SOURCE_DIR}/extfilt.h
            ${PROJECT_SOURCE_DIR}/filter.h
            ${PROJECT_SOURCE_DIR}/pot.h
            ${PROJECT_SOURCE_DIR}/sid.h
            ${PROJECT_SOURCE_DIR}/siddefs.h
            ${PROJECT_SOURCE_DIR}/spline.h
            ${PROJECT_SOURCE_DIR}/voice.h
            ${PROJECT_SOURCE_DIR}/wave.h)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/resid)
install(TARGETS resid DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
