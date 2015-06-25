/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spark/Result.h>
#include <spark/Types.h>
#include <spark/Link.h>
#include "PlayerDamage.h"
#include <cstdint>

namespace ember {

class Player {
	std::uint32_t id_;
	std::uint32_t /*spark::ServerID*/ server_id_;

	std::uint32_t health_ = 100;

public:
	Player();

	spark::Result Player::DamageHandler(const spark::Link& link, const PlayerDamage* damage);

	std::uint32_t /*spark::ServerID*/ server_id() {
		return server_id_;
	}

	std::uint32_t id() {
		return id_;
	}
};

} // ember