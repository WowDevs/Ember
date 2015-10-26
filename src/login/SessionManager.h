/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "NetworkSession.h"
#include <memory>
#include <set>

namespace ember {

class SessionManager {
	std::set<std::shared_ptr<NetworkSession>> sessions_;

public:
	void start(std::shared_ptr<NetworkSession> session);
	void stop(std::shared_ptr<NetworkSession> session);
	void stop_all();
};

} // ember