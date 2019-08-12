#  -------------------------------------------------------------------------
#  Copyright (C) 2019 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

find_package(Threads REQUIRED)
if (CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

MACRO(INSTALL_RESOURCES files)
   SET(targetFiles "")
   FOREACH(file ${files})
       set(source ${CMAKE_CURRENT_SOURCE_DIR}/${file})
       set(target ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/${file})
       
       add_custom_command(
          OUTPUT ${target}
          COMMAND ${CMAKE_COMMAND} -E echo "Copy file ${file} to build directory"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source}" "${target}"
          DEPENDS ${file})
       LIST(APPEND targetFiles "${target}")
   ENDFOREACH()
   
   set(targetName copy-files-${CMAKE_CURRENT_SOURCE_DIR})
   string(REPLACE "/" "-" targetName ${targetName})
   string(REPLACE ":" "-" targetName ${targetName})
   add_custom_target(${targetName} ALL DEPENDS ${targetFiles})
   
   install(FILES ${targetFiles} DESTINATION bin/res)
ENDMACRO()

include_directories(ramses-citymodel/include)
add_subdirectory(ramses-citymodel)
add_subdirectory(ramses-citymodel-client)
add_subdirectory(ramses-citymodel-renderer)
add_subdirectory(res)
