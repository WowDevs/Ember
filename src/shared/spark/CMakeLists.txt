# Copyright (c) 2015 Ember
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(spark
            src/Server.cpp
            include/spark/Server.h
            include/spark/Handler.h
            include/spark/Link.h
            include/spark/Message.h
            include/spark/Result.h
            include/spark/RouteConfig.h
            include/spark/Types.h
            include/spark/BufferChain.h
            src/BufferChain.cpp
            include/spark/BufferChainNode.h
            src/BufferChainNode.cpp
)

target_include_directories(spark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
