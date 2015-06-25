/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "TestService.h"
#include "TestProtocol.h"
#include <spark/RouteConfig.h>

namespace ember {

TestService::TestService(spark::Server& server) : server_(server) {
	configure();
}

void TestService::configure() {
	spark::RouteConfig& config = server_.route_config();
	config.configure_inbound<TestProtocol>();
	config.configure_outbound<TestProtocol>();
}

} // ember