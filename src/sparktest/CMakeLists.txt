# Copyright (c) 2015 Ember
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(EXECUTABLE_NAME sparktest)

set(EXECUTABLE_HDR
    Player.h
    TestService.h
    TestProtocol.h
    PlayerDamage.h
    )

set(EXECUTABLE_SRC
    main.cpp
    Player.cpp
    TestService.cpp
    )

include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_HDR} ${EXECUTABLE_SRC} ${version_file})
target_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME} spark)
INSTALL(TARGETS ${EXECUTABLE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})