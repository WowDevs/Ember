/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <shared/database/Exception.h>
#include <shared/database/objects/User.h>
#include <boost/optional.hpp>
#include <string>

namespace ember { namespace dal {

class UserDAO {
public:
	virtual boost::optional<User> user(const std::string& username) const = 0;
	virtual void record_last_login(const User& user, const std::string& ip) const = 0;
	virtual ~UserDAO() = default;
};

}} //dal, ember