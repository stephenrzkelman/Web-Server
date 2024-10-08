//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "config_parser.h"
#include "logging.h"
#include "request_manager.h"
#include "server.h"
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <iostream>

namespace logging = boost::log;
namespace src = boost::log::sources;

int main(int argc, char *argv[]) {
  using namespace logging::trivial;

  // Initialize logging, Demonstrate logging capability.
  init_logging();
  BOOST_LOG_TRIVIAL(info) << "Logging Initialized, starting server executable";

  // TODO: add logs to the logic here.
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <config_file>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    // Parse config
    NginxConfigParser config_parser;
    NginxConfig config;
    bool successful_parse = config_parser.Parse(argv[1], &config);
    if (!successful_parse) {
      BOOST_LOG_TRIVIAL(error) << "Config failed to parse & validate";
      throw("Config failed to parse or is invalid");
    }
    int port_number = config.findPort();
    if (port_number == -1) {
      BOOST_LOG_TRIVIAL(error) << "Port not provided properly";
      throw("Port number not provided properly");
    }

    // Setup request manager
    std::optional<std::unordered_map<std::string, LocationData>> locations =
        config.findLocations();
    if (!locations.has_value()) {
      BOOST_LOG_TRIVIAL(error) << "Failed to extract location data from config";
      throw("Failed to extract location data from config");
    }
    RequestManager request_manager = RequestManager(locations.value());

    // Setup server
    server s(io_service, request_manager, port_number);
    s.start_accept();

    // Run io with thread pool
    boost::thread_group tg;
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    }
    tg.join_all();

  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
