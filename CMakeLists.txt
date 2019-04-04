#  -------------------------------------------------------------------------
#  Copyright (C) 2019 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.3)

set(ramses-sdk_BUILD_EXAMPLES OFF CACHE BOOL "")
set(ramses-sdk_BUILD_TESTS OFF CACHE BOOL "")
set(ramses-sdk_BUILD_SMOKE_TESTS OFF CACHE BOOL "")
set(ramses-sdk_BUILD_DEMOS OFF CACHE BOOL "")
set(ramses-sdk_ENABLE_DLT OFF CACHE BOOL "")
set(ramses-sdk_INSTALL_STATIC_LIBS OFF CACHE BOOL "")
set(ramses-sdk_utils OFF CACHE BOOL "")
set(ramses-sdk_external_wayland-ivi-example-client OFF CACHE BOOL "")

add_subdirectory(ramses)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/ramses/client/ramses-client/ramses-client-api/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ramses/client/ramses-text/ramses-text-api/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ramses/framework/ramses-framework-api/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ramses/renderer/RendererLib/ramses-renderer-api/include    
)

set(OpenCTM_DIR OpenCTM-1.0.3)
file(GLOB_RECURSE libsrc "${OpenCTM_DIR}/lib/*.c")
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/${OpenCTM_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/${OpenCTM_DIR}/lib/liblzma)
add_library(OpenCTM ${libsrc})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include)

add_subdirectory(ramses-citymodel-demo)
