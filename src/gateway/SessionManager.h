/*
 * Copyright (c) 2015-2016 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <cstddef>

namespace ember {

class ClientConnection;
struct ConnectionStats;

class SessionManager {
	std::set<std::shared_ptr<ClientConnection>> sessions_;
	mutable std::mutex sessions_lock_;

public:
	void start(std::shared_ptr<ClientConnection> session);
	void stop(std::shared_ptr<ClientConnection> session);
	void stop_all();
	std::size_t count() const;
	ConnectionStats aggregate_stats() const;
};

} // ember