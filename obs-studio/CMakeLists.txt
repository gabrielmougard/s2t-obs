# TODO: write conditions/variables for cross-compilation with linux 
# or different architectures like ARM for example

cmake_minimum_required(VERSION 3.9)
project(obs-studio)

include(ExternalProject)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(OBS_TARGET obs-studio)
set(OBS_DEPS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/obs-build-dependencies)
set(OBS_STUDIO_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/obs-studio)

execute_process(COMMAND uname -m OUTPUT_VARIABLE UNAME)

if(APPLE)
  set(CEF_ROOT_DIR ${OBS_DEPS_BUILD_DIR}/cef_binary_4638_macos_x86_64)
  if(UNAME STREQUAL x86_64)
    set(OBS_DEPLOYMENT_TARGET
  "-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.13")
  else()
    set(OBS_DEPLOYMENT_TARGET
  "-DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=11.0")
  endif()
  set(OBS_OSX_ARCH "-DCMAKE_OSX_ARCHITECTURES:STRING=x86_64")

  ExternalProject_Add(
    ${OBS_TARGET}
    PREFIX obs-studio
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/obs-studio"
    CMAKE_CACHE_ARGS -DCEF_ROOT_DIR:PATH=${CEF_ROOT_DIR}
                     -DENABLE_BROWSER:BOOL=ON
                     -DVLC_PATH:PATH=${OBS_DEPS_BUILD_DIR}/vlc-3.0.8
                     -DENABLE_VLC:BOOL=ON
                     -DCMAKE_PREFIX_PATH:PATH=${OBS_DEPS_BUILD_DIR}/obs-deps
                     -DBROWSER_LEGACY:BOOL=OFF
                     ${OBS_DEPLOYMENT_TARGET}
                     ${OBS_OSX_ARCH}
                     -DOBS_CODESIGN_LINKER:BOOL=OFF
                     -DCMAKE_INSTALL_PREFIX:PATH=${OBS_STUDIO_INSTALL_DIR}
                     -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
  )
  
elseif(UNIX AND NOT APPLE)
  set(CEF_ROOT_DIR ${OBS_DEPS_BUILD_DIR}/cef_binary_4638_linux64)
  ExternalProject_Add(
    ${OBS_TARGET}
    PREFIX obs-studio
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/obs-studio"
    CMAKE_CACHE_ARGS -DCEF_ROOT_DIR:PATH=${CEF_ROOT_DIR}
                     -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo
                     -DCMAKE_INSTALL_PREFIX:PATH=${OBS_STUDIO_INSTALL_DIR}
                     -DLINUX_PORTABLE:BOOL=OFF
                     -DENABLE_AJA:BOOL=OFF
  )
else()
  message(FATAL_ERROR "OBS studio build for s2t-plugin not yet supported for windows platforms")
endif()

configure_package_config_file(cmake/obs-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/obs-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/libobs
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "obs-hevc.h" EXCLUDE
        PATTERN "*-windows.h" EXCLUDE
        PATTERN "*-x11.h" EXCLUDE
        PATTERN "*-wayland.h" EXCLUDE
        PATTERN "audio-monitoring/null" EXCLUDE
        PATTERN "audio-monitoring/win32" EXCLUDE
        PATTERN "audio-monitoring/pulse" EXCLUDE
        PATTERN "util/windows" EXCLUDE
        PATTERN "cmake" EXCLUDE
        PATTERN "pkgconfig" EXCLUDE
        PATTERN "data" EXCLUDE
)

install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/UI/obs-frontend-api/obs-frontend-api.h
        ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/UI/obs-frontend-api/obs-frontend-internal.hpp
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/obs-frontend-api
)

# Install libobs targets
install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/build/libobs/libobsConfig.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/build/libobs/libobsConfigVersion.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/build/libobs/libobsTargets.cmake
          DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/libobs
)

# Install obs-frontend-api targets
install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/build/UI/obs-frontend-api/obs-frontend-apiConfig.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/build/UI/obs-frontend-api/obs-frontend-apiConfigVersion.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/obs-studio/build/UI/obs-frontend-api/obs-frontend-apiTargets.cmake
          DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/obs-frontend-api
)
