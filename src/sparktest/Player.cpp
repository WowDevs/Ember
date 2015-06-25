/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Player.h"
#include <iostream>

namespace ember {

Player::Player() {
	
}

spark::Result Player::DamageHandler(const spark::Link& link, const PlayerDamage* damage) {
	health_ -= damage->amount;
	std::cout << "Player damaged by " << damage->amount << "\n";
	return spark::Result::OK;
}

} // ember