/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "TestService.h"
#include <spark/BufferChain.h>
#include <spark/BinaryDecoder.h>
#include <spark/Server.h>
#include <iostream>
#include <string>

using namespace ember;

void launch();

int main() {
	launch();
}

struct C {
	int a;
	int b;
};
 
void launch() try {
	spark::Server server;
	TestService test_service(server);
	spark::BufferChain<> chain;
	spark::BinaryStream<spark::BufferChain<>> stream(chain);
	C foo;
	stream << C{ 5, 10 };
	stream << C{ 10, 20 };
	stream >> foo;
	std::cout << foo.b;
	stream >> foo;
	std::cout << foo.b;
} catch(std::exception& e) {
	std::cout << e.what();
}
