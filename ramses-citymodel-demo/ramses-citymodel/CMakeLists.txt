#  -------------------------------------------------------------------------
#  Copyright (C) 2019 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB libsrc "src/*.cpp")
add_library(ramses-citymodel ${libsrc})
target_link_libraries(ramses-citymodel OpenCTM lz4 ramses-client ramses-text)
