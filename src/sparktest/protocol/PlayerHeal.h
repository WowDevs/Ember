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

class PlayerHeal : public spark::Message {
	static const std::uint32_t opcode_ = 0x100;

public:
	std::uint32_t healed_by;
	std::uint32_t target;
	std::uint32_t amount;

	void serialise(spark::BinaryStream& stream) const;
	void deserialise(spark::BinaryStream& stream);
	std::uint32_t get_opcode() { return opcode_; }
};

} // ember