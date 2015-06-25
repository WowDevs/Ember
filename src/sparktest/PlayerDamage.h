/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spark/Message.h>
#include <spark/Types.h>
#include <cstdint>

namespace ember {

struct PlayerDamage : spark::Message {
	std::uint16_t amount = 10;
};

} // ember