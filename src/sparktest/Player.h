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
#include <cstdint>

namespace ember {

class Player {
	std::uint32_t id_;
	std::uint32_t server_id_;

	std::uint32_t health_ = 100;

public:
	void cast_heal(std::uint32_t player_id, std::uint32_t amount);
	void attack(std::uint32_t player_id, std::uint32_t amount);

	spark::Result damage_handler(const spark::Link& link, const PlayerDamage* msg);
	spark::Result heal_handler(const spark::Link& link, const PlayerHeal* msg);
};

} // ember