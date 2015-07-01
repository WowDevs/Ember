/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <spark/BinaryStream.h>
#include <cstdint>
#include <cstddef>

namespace ember { namespace spark {

class Message {
public:
	virtual std::uint32_t get_opcode() = 0;
};

}} // spark, ember