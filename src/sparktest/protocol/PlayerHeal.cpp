/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "PlayerHeal.h"
#include <cstdint>

namespace ember {

void PlayerHeal::serialise(spark::BinaryStream& stream) const {
	stream << opcode_;
	stream << healed_by;
	stream << target;
	stream << amount;
}

void PlayerHeal::deserialise(spark::BinaryStream& stream) {
	stream >> healed_by;
	stream << target;
	stream >> amount;
}

} // ember