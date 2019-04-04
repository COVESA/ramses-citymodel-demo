#  -------------------------------------------------------------------------
#  Copyright (C) 2019 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

set(rendererConfigList
    "x11-egl-es-3-0"

    "wayland-ivi-egl-es-3-0"
    "wayland-shell-egl-es-3-0"

    "windows-wgl-es-3-0"
    "windows-wgl-4-2-core"
    "windows-wgl-4-5"
    )
    
file(GLOB libsrc "src/*.cpp")

foreach(platformName ${rendererConfigList})
    set(ramsesSharedLibName ramses-shared-lib-${platformName})
    if (TARGET ${ramsesSharedLibName})
        set(executableName ramses-citymodel-renderer-${platformName})
        add_executable(${executableName} ${libsrc})
        target_link_libraries(${executableName} ramses-citymodel ${ramsesSharedLibName})
        install(TARGETS ${executableName} DESTINATION bin)
    endif()
endforeach()

