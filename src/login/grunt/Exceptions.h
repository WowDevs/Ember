/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdexcept>
#include <string>
 
namespace ember { namespace grunt {

class exception : public std::runtime_error {
public:
	exception() : std::runtime_error("An unknown Grunt error occured!") { }
	explicit exception(std::string msg) : std::runtime_error(msg) { };
};

class bad_packet : public exception {
public:
	bad_packet() : exception("Bad packet encountered!") { }
	explicit bad_packet(std::string msg) : exception(msg) { };
};

}} // grunt, ember