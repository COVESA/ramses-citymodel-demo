 #  -------------------------------------------------------------------------
#  Copyright (C) 2019 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
 
 file(GLOB filesWithPath *.frag *.vert *.rex *.png *.ttf)
 FOREACH(fileWithPath ${filesWithPath})
    get_filename_component(file ${fileWithPath} NAME)
    list(APPEND files ${file})
 ENDFOREACH()
 
 INSTALL_RESOURCES("${files}")