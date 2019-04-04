#  -------------------------------------------------------------------------
#  Copyright (C) 2019 Mentor Graphics Development GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

file(GLOB libsrc "src/*.cpp")
add_executable(ramses-citymodel-client ${libsrc})
target_link_libraries(ramses-citymodel-client ramses-citymodel Threads::Threads)

install(TARGETS ramses-citymodel-client DESTINATION bin)
