/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "TestService.h"
#include <spark/BufferChain.h>
#include <spark/Server.h>
#include <iostream>
#include <string>

using namespace ember;

void launch();

int main() {
	launch();
}

void launch() try {
	spark::Server server;
	TestService test_service(server);
	spark::BufferChain<9> chain;
	char foo[5];
	
	std::string test("abcdefghijklmnopqrstuvwxyz");

	chain.write(test.c_str(), test.length());

	char test2 = 'b';
	chain.write(&test2, 1);

	chain.read(foo, sizeof(foo));
	std::cout << std::string(foo, sizeof(foo));
	
	auto buffers = chain.fetch_buffers(22);
	for(auto& buffer : buffers) {
		std::cout << std::string(buffer.first, buffer.second);
	}

} catch(std::exception& e) {
	std::cout << e.what();
}
