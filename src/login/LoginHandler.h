/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Actions.h"
#include "AccountService.h"
#include "Authenticator.h"
#include "GameVersion.h"
#include "RealmList.h"
#include "grunt/Packets.h"
#include "grunt/Handler.h"
#include <logger/Logging.h>
#include <shared/database/daos/UserDAO.h>
#include <shared/metrics/Metrics.h>
#include <botan/bigint.h>
#include <boost/optional.hpp>
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace ember {

class Patcher;
class NetworkSession;
class Metrics;

class LoginHandler {
	enum class State {
		INITIAL_CHALLENGE, LOGIN_PROOF, RECONNECT_PROOF, REQUEST_REALMS,
		FETCHING_USER_LOGIN, FETCHING_USER_RECONNECT, FETCHING_SESSION,
		WRITING_SESSION, CLOSED
	};

	State state_ = State::INITIAL_CHALLENGE;
	Metrics& metrics_;
	log::Logger* logger_;
	const Patcher& patcher_;
	const RealmList& realm_list_;
	const dal::UserDAO& user_src_;
	boost::optional<User> user_;
	Botan::BigInt server_proof_;
	const std::string source_;
	const AccountService& acct_svc_;
	std::unique_ptr<LoginAuthenticator> login_auth_;
	std::unique_ptr<ReconnectAuthenticator> reconn_auth_;

	void send_realm_list(const grunt::Packet* packet);
	void process_challenge(const grunt::Packet* packet);
	void check_login_proof(const grunt::Packet* packet);
	void send_reconnect_proof(const grunt::Packet* packet);
	void send_login_proof_failure(grunt::ResultCode result);
	void build_login_challenge(grunt::server::LoginChallenge* packet);
	void send_login_challenge(FetchUserAction* action);
	void send_login_proof(RegisterSessionAction* action);
	void send_reconnect_challenge(FetchSessionKeyAction* action);

	void fetch_user(grunt::Opcode opcode, const std::string& username);
	void fetch_session_key(FetchUserAction* action);
	void reject_client(const GameVersion& version);
	void patch_client(const GameVersion& version);

public:
	std::function<void(std::shared_ptr<Action> action)> execute_async;
	std::function<void(grunt::PacketHandle)> send;

	bool update_state(std::shared_ptr<Action> action);
	bool update_state(const grunt::Packet* packet);

	LoginHandler(const dal::UserDAO& users, const AccountService& acct_svc, const Patcher& patcher,
	             log::Logger* logger, const RealmList& realm_list, std::string source, Metrics& metrics)
	             : user_src_(users), patcher_(patcher), logger_(logger), acct_svc_(acct_svc),
	               realm_list_(realm_list), source_(std::move(source)), metrics_(metrics) {}
};

} // ember