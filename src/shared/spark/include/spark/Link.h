/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <spark/Types.h>
#include <string>

namespace ember { namespace spark {

class Link {
	const std::string name_;
	const ServerID server_id_;

public:
	Link(std::string name) : name_(std::move(name)), server_id_(5) {}

	std::string who() {
		return name_;
	}
};

}} // spark, ember