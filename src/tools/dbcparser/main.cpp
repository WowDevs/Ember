/*
 * Copyright (c) 2014 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Parser.h"
#include "Validator.h"
#include "bprinter/table_printer.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace edbc = ember::dbc;

po::variables_map parse_arguments(int argc, const char* argv[]);
std::vector<std::string> fetch_definitions(const std::string& path);
void print_dbc_table(const std::vector<edbc::Definition>& defs);
void print_dbc_fields(const std::string& dbc, const std::vector<edbc::Definition>& defs);

int main(int argc, const char* argv[]) try {
	const po::variables_map args = parse_arguments(argc, argv);
	const std::string def_path = args["definitions"].as<std::string>();
	std::vector<std::string> paths = fetch_definitions(def_path);
	
	edbc::Parser parser;
	std::vector<edbc::Definition> definitions = parser.parse(paths);
	edbc::Validator validator(definitions);
	validator.validate();

	if(args.count("print-dbcs")) {
		print_dbc_table(definitions);
	}

	if(args.count("print-fields")) {
		print_dbc_fields(args["print-fields"].as<std::string>(), definitions);
	}
} catch(std::exception& e) {
	std::cerr << e.what();
	return 1;
}

void print_dbc_table(const std::vector<edbc::Definition>& defs) {
	bprinter::TablePrinter printer(&std::cout);
	printer.AddColumn("DBC Name", 26);
	printer.AddColumn("#", 4);
	printer.AddColumn("Comment", 45);
	printer.PrintHeader();

	for(auto& d : defs) {
		printer << d.dbc_name.substr(0, 26) << d.fields.size() << d.comment;
	}
}

void print_dbc_fields(const std::string& dbc, const std::vector<edbc::Definition>& defs) {
	auto def = std::find_if(defs.begin(), defs.end(), [dbc](const edbc::Definition& def) {
		return dbc == def.dbc_name;
	});
	
	if(def == defs.end()) {
		throw std::invalid_argument(dbc + " - no such definition to print");
	}

	bprinter::TablePrinter printer(&std::cout);
	printer.AddColumn("Field", 32);
	printer.AddColumn("Type", 18);
	printer.AddColumn("Key", 4);
	printer.AddColumn("Comment", 20);
	printer.PrintHeader();

	for(auto& f : def->fields) {
		std::string key;

		switch(f.keys.size()) {
			case 1:
				key = f.keys[0].type.data()[0];
				break;
			case 2:
				key = "pf";
				break;
		}

		printer << f.name << f.type << key << f.comment;
	}
}

std::vector<std::string> fetch_definitions(const std::string& path) {
	if(!boost::filesystem::is_directory(path)) {
		throw std::exception("Invalid path provided.");
	}

	std::vector<std::string> paths;

	for(auto& file : fs::directory_iterator(path)) {
		if(file.path().extension() == ".xml") {
			paths.emplace_back(file.path().string());
		}
	}

	return paths;
}

po::variables_map parse_arguments(int argc, const char* argv[]) {
	po::options_description opt("Generic options");
	opt.add_options()
		("help", "Displays a list of available options")
		("definitions,d", po::value<std::string>()->default_value("definitions"),
			"Path to the DBC XML definitions")
		("headers,h", po::bool_switch()->default_value(false),
			"Generate header files")
		("sql-schema,s", po::bool_switch()->default_value(false),
			"Generate SQL data")
		("sql-inserts,i", po::bool_switch()->default_value(false),
			"Generate SQL data")
		("print-dbcs", po::bool_switch()->default_value(false), //file bug Boost report
			"Print out a summary of the DBC definitions in a table")
		("print-fields", po::value<std::string>(),
			"Print out of a summary of a specific DBC definition's fields");

	po::variables_map options;
	po::store(po::command_line_parser(argc, argv).options(opt)
	          .style(po::command_line_style::default_style & ~po::command_line_style::allow_guessing)
	          .run(), options);
	po::notify(options);

	if(options.count("help")) {
		std::cout << opt << "\n";
		std::exit(0);
	}

	return std::move(options);
}