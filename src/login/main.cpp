/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Authenticator.h"
#include "GameVersion.h"
#include "LoginHandlerBuilder.h"
#include "LoginPacketCheck.h"
#include "NetworkHandler.h"
#include "Patcher.h"
#include "RealmList.h"
#include "RealmHandler.h"
#include "RealmPacketCheck.h"
#include "PatchCache.h"
#include <logger/Logging.h>
#include <conpool/ConnectionPool.h>
#include <conpool/Policies.h>
#include <conpool/drivers/AutoSelect.h>
#include <shared/Banner.h>
#include <shared/Version.h>
#include <shared/util/LogConfig.h>
#include <shared/memory/ASIOAllocator.h>
#include <shared/threading/ThreadPool.h>
#include <shared/database/daos/IPBanDAO.h>
#include <shared/database/daos/RealmDAO.h>
#include <shared/database/daos/UserDAO.h>
#include <shared/IPBanCache.h>
#include <botan/init.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace el = ember::log;
namespace ep = ember::connection_pool;
namespace po = boost::program_options;
namespace ba = boost::asio;

std::vector<ember::GameVersion> client_versions();
unsigned int check_concurrency(el::Logger* logger);
void launch(const po::variables_map& args, el::Logger* logger);
po::variables_map parse_arguments(int argc, const char* argv[]);
ember::drivers::MySQL init_db_driver(const po::variables_map& args);
std::unique_ptr<ember::log::Logger> init_logging(const po::variables_map& args);
void pool_log_callback(ep::Severity, const std::string& message, el::Logger* logger);
void start_workers(unsigned int concurrency, ba::io_service& service, el::Logger* logger);

/*
 * We want to do the minimum amount of work required to get 
 * logging facilities and crash handlers up and running in main.
 *
 * Exceptions that aren't derived from std::exception are
 * left to the crash handler since we can't get useful information
 * from them.
 */
int main(int argc, const char* argv[]) try {
	ember::print_banner("Login Daemon");
	const po::variables_map args = parse_arguments(argc, argv);

	auto logger = ember::util::init_logging(args);
	el::set_global_logger(logger.get());
	LOG_INFO(logger) << "Logger configured successfully" << LOG_SYNC;

	launch(args, logger.get());
} catch(std::exception& e) {
	std::cerr << e.what();
	return 1;
}

void launch(const po::variables_map& args, el::Logger* logger) try {
	LOG_INFO(logger) << "Initialialising Botan..." << LOG_SYNC;
	Botan::LibraryInitializer init("thread_safe");

	auto driver(init_db_driver(args));
	auto min_conns = args["database.min_connections"].as<unsigned short>();
	auto max_conns = args["database.max_connections"].as<unsigned short>();
	
	LOG_INFO(logger) << "Compiled with " << driver.name() << " (" << driver.version() << ")" << LOG_SYNC;
	LOG_INFO(logger) << "Initialising database connection pool..."<< LOG_SYNC;
	ep::Pool<decltype(driver), ep::CheckinClean, ep::ExponentialGrowth>
		pool(driver, min_conns, max_conns, std::chrono::seconds(30));
	pool.logging_callback(std::bind(pool_log_callback, std::placeholders::_1,
	                                std::placeholders::_2, logger));
	
	LOG_INFO(logger) << "Initialising DAOs..." << LOG_SYNC;
	auto user_dao = ember::dal::user_dao(pool);
	auto realm_dao = ember::dal::realm_dao(pool);
	auto ip_ban_dao = ember::dal::ip_ban_dao(pool);
	auto ip_ban_cache = ember::IPBanCache<ember::dal::IPBanDAO>(*ip_ban_dao);

	LOG_INFO(logger) << "Loading realm list..." << LOG_SYNC;
	ember::RealmList realm_list(realm_dao->get_realms());

	unsigned int concurrency = check_concurrency(logger);

	LOG_INFO(logger) << "Starting thread pool with " << concurrency << " threads..." << LOG_SYNC;
	ember::ThreadPool thread_pool(concurrency);

	LOG_INFO(logger) << "Initialising patch cache..." << LOG_SYNC;
	const auto allowed_clients = client_versions();
	ember::Patcher patcher(allowed_clients, ember::patch_cache::fetch("patches"));
	
	if(args["survey.enabled"].as<bool>()) {

	}

	// Start login server
	auto interface = args["network.interface"].as<std::string>();
	auto port = args["network.port"].as<unsigned short>();
	ember::LoginHandlerBuilder builder(logger, patcher, *user_dao, realm_list);

	ba::io_service service(concurrency);

	LOG_INFO(logger) << "Binding server to " << interface << ":" << port << LOG_SYNC;
	ember::NetworkHandler<ember::LoginHandler> login_server(service, port, interface,
		ip_ban_cache, thread_pool, logger,
		std::bind(&ember::LoginHandlerBuilder::create, builder, std::placeholders::_1),
		std::bind(&ember::protocol::check_packet_completion, std::placeholders::_1));

	// Start realm listener
	auto r_interface = args["realm_listen.interface"].as<std::string>();
	auto r_port = args["realm_listen.port"].as<unsigned short>();

	/*LOG_INFO(logger) << "Binding realm listener to " << r_interface << ":" << r_port << LOG_SYNC;
	ember::NetworkHandler<ember::RealmHandler> realm_server(service, port, interface,
		ip_ban_cache, thread_pool, logger,
		std::bind(&ember::LoginHandlerBuilder::create, builder, std::placeholders::_1),
		std::bind(&ember::protocol::check_packet_completion, std::placeholders::_1));*/

	ba::signal_set signals(service, SIGINT, SIGTERM);
	signals.async_wait(std::bind(&ba::io_service::stop, &service));

	start_workers(concurrency, service, logger);

	LOG_INFO(logger) << "Login daemon shutting down..." << LOG_SYNC;
} catch(std::exception& e) {
	LOG_FATAL(logger) << e.what() << LOG_SYNC;
}

/*
 * Start the worker threads that are responsible for handling login requests.
 * This function blocks until all worker threads have finished, which should only
 * happen when the login server receives a termination signal.
 */
void start_workers(unsigned int concurrency, ba::io_service& service, el::Logger* logger) {
	std::vector<std::thread> workers;

	for(unsigned int i = 0; i < concurrency; ++i) {
		workers.emplace_back(static_cast<std::size_t(ba::io_service::*)()>
			(&ba::io_service::run), &service); 
	}

	LOG_INFO(logger) << "Launched " << concurrency << " login workers" << LOG_SYNC;
	LOG_INFO(logger) << "Login daemon started successfully" << LOG_SYNC;

	for(auto& w : workers) {
		w.join();
	}
}

/* 
 * This vector defines the client builds that are allowed to connect to the
 * server. All builds in this list should be using the same protocol version.
 */
std::vector<ember::GameVersion> client_versions() {
	return {{1, 12, 1, 5875}, {1, 12, 2, 6005}};
}

po::variables_map parse_arguments(int argc, const char* argv[]) {
	//Command-line options
	po::options_description cmdline_opts("Generic options");
	cmdline_opts.add_options()
		("help", "Displays a list of available options")
		("config,c", po::value<std::string>()->default_value("login.conf"),
			"Path to the configuration file");

	po::positional_options_description pos; 
	pos.add("config", 1);

	//Config file options
	po::options_description config_opts("Login configuration options");
	config_opts.add_options()
		("network.interface,", po::value<std::string>()->default_value("0.0.0.0"))
		("network.port", po::value<unsigned short>()->default_value(3724))
		("realm_listen.interface,", po::value<std::string>()->default_value("0.0.0.0"))
		("realm_listen.port", po::value<unsigned short>()->default_value(3749))
		("console_log.verbosity,", po::value<std::string>()->required())
		("remote_log.verbosity,", po::value<std::string>()->required())
		("remote_log.service_name,", po::value<std::string>()->required())
		("remote_log.host,", po::value<std::string>()->required())
		("remote_log.port,", po::value<unsigned short>()->required())
		("realmbridge.host,", po::value<std::string>()->required())
		("realmbridge.port,", po::value<unsigned short>()->required())
		("file_log.verbosity,", po::value<std::string>()->required())
		("file_log.path,", po::value<std::string>()->default_value("login.log"))
		("file_log.timestamp_format,", po::value<std::string>())
		("file_log.mode,", po::value<std::string>()->required())
		("file_log.size_rotate,", po::value<std::uint32_t>()->required())
		("file_log.midnight_rotate,", po::bool_switch()->required())
		("file_log.log_timestamp,", po::bool_switch()->required())
		("file_log.log_severity,", po::bool_switch()->required())
		("database.username", po::value<std::string>()->required())
		("database.password", po::value<std::string>()->default_value(""))
		("database.database", po::value<std::string>()->required())
		("database.host", po::value<std::string>()->required())
		("database.port", po::value<unsigned short>()->required())
		("database.min_connections", po::value<unsigned short>()->required())
		("database.max_connections", po::value<unsigned short>()->required());

	po::variables_map options;
	po::store(po::command_line_parser(argc, argv).positional(pos).options(cmdline_opts).run(), options);
	po::notify(options);

	if(options.count("help")) {
		std::cout << cmdline_opts << "\n";
		std::exit(0);
	}

	std::string config_path = options["config"].as<std::string>();
	std::ifstream ifs(config_path);

	if(!ifs) {
		std::string message("Unable to open configuration file: " + config_path);
		throw std::invalid_argument(message);
	}

	po::store(po::parse_config_file(ifs, config_opts), options);
	po::notify(options);

	return std::move(options);
}

/*
 * The concurrency level returned is usually the number of logical cores
 * in the machine but the standard doesn't guarantee that it won't be zero.
 * In that case, we just set the minimum concurrency level to two.
 */
unsigned int check_concurrency(el::Logger* logger) {
	unsigned int concurrency = std::thread::hardware_concurrency();

	if(!concurrency) {
		concurrency = 2;
		LOG_WARN(logger) << "Unable to determine concurrency level" << LOG_SYNC;
	}

	return concurrency;
}

ember::drivers::DriverType init_db_driver(const po::variables_map& args) {
	auto user = args["database.username"].as<std::string>();
	auto pass = args["database.password"].as<std::string>();
	auto host = args["database.host"].as<std::string>();
	auto port = args["database.port"].as<unsigned short>();
	auto db = args["database.database"].as<std::string>();
	return {user, pass, host, port, db};
}

void pool_log_callback(ep::Severity severity, const std::string& message, el::Logger* logger) {
	switch(severity) {
		case(ep::Severity::DEBUG):
			LOG_DEBUG(logger) << message << LOG_ASYNC;
			break;
		case(ep::Severity::INFO):
			LOG_INFO(logger) << message << LOG_ASYNC;
			break;
		case(ep::Severity::WARN):
			LOG_WARN(logger) << message << LOG_ASYNC;
			break;
		case(ep::Severity::ERROR):
			LOG_ERROR(logger) << message << LOG_ASYNC;
			break;
		case(ep::Severity::FATAL):
			LOG_FATAL(logger) << message << LOG_ASYNC;
			break;
		default:
			LOG_ERROR(logger) << "Unhandled pool log callback severity" << LOG_ASYNC;
			LOG_ERROR(logger) << message << LOG_ASYNC;
	}	
}