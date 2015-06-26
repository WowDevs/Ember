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
	spark::BufferChain<1023> chain;
	
	char bar = 'a';

	for(int i = 0; i < 1023; ++i) {
		chain.write(&bar, sizeof(bar));
	}

	char test = 'b';
	chain.write(&test, 1);

	auto buffers = chain.fetch_buffers(1024);

	for(auto& buffer : buffers) {
		std::cout << std::string(buffer.first, buffer.second);
	}

	//std::cout << std::string(foo, 1024);
} catch(std::exception& e) {
	std::cout << e.what();
}
