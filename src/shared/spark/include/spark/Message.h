/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace ember { namespace spark {

class Message {
	std::uint16_t code_ = 5;
	std::size_t temp;
public:
	std::uint16_t code() {
		return code_;
	}
};

}} // spark, ember