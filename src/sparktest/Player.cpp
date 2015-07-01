/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "protocol/PlayerHeal.h"
#include "protocol/PlayerDamage.h"
#include "Player.h"
#include <iostream>

namespace ember {

void Player::cast_heal(std::uint32_t player_id, std::uint32_t amount) {
	PlayerHeal msg;
	msg.healed_by = id_;
	msg.target = player_id;
	msg.amount = amount;
}

void Player::attack(std::uint32_t player_id, std::uint32_t amount) {
	PlayerDamage msg;
	msg.attacked_by = id_;
	msg.target = player_id;
	msg.amount = amount;
}

spark::Result Player::heal_handler(const spark::Link& link, const PlayerHeal* msg) {
	health_ += msg->amount;
	std::cout << "Player healed by " << msg->amount << "\n";
	return spark::Result::OK;
}

spark::Result Player::damage_handler(const spark::Link& link, const PlayerDamage* msg) {
	health_ -= msg->amount;
	std::cout << "Player damaged by " << msg->amount << "\n";
	return spark::Result::OK;
}

} // ember