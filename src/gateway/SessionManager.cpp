/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SessionManager.h"
#include "ClientConnection.h"
#include "ConnectionStats.h"

namespace ember {

void SessionManager::start(std::shared_ptr<ClientConnection> session) {
	std::lock_guard<std::mutex> guard(sessions_lock_);
	sessions_.insert(session);
	session->start();
}

void SessionManager::stop(std::shared_ptr<ClientConnection> session) {
	std::lock_guard<std::mutex> guard(sessions_lock_);
	sessions_.erase(session);
	session->stop();
}

void SessionManager::stop_all() {
	for(auto& session : sessions_) {
		session->stop();
	}

	std::lock_guard<std::mutex> guard(sessions_lock_);
	sessions_.clear();
}

std::size_t SessionManager::count() const {
	return sessions_.size();
}

ConnectionStats SessionManager::aggregate_stats() const {
	std::lock_guard<std::mutex> guard(sessions_lock_);
	ConnectionStats ag_stats;

	for(auto& session : sessions_) {
		auto stats = session->stats();
		ag_stats.bytes_in += stats.bytes_in;
		ag_stats.bytes_out += stats.bytes_out;
		ag_stats.latency += stats.latency;
		ag_stats.messages_in += stats.messages_in;
		ag_stats.messages_out += stats.messages_out;
		ag_stats.packets_in += stats.packets_in;
		ag_stats.packets_out += stats.packets_out;
	}

	ag_stats.latency /= count(); // average latency
	return ag_stats;
}


} // ember