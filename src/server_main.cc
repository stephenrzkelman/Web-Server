//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include "logging.h"
#include "config_parser.h"
#include "session.h"
#include "server.h"
#include "request_handler.h"
#include "echo_request_handler.h"
#include "content_request_handler.h"
#include "error_request_handler.h"
#include "request_manager.h"
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;

int main(int argc, char* argv[])
{
  using namespace logging::trivial;

  // Initialize logging, Demonstrate logging capability.
  init_logging();
  BOOST_LOG_TRIVIAL(info) << "Logging Initialized, starting server executable";

  // TODO: add logs to the logic here.
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <config_file>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    NginxConfigParser config_parser;
    NginxConfig config;
    bool successful_parse = config_parser.Parse(argv[1], &config);
    if(!successful_parse){
      throw ("Failed to parse config");
    }
    bool valid_config = config.Validate();
    if(!valid_config){
      throw ("Config contains unrecognized directive or subcontext");
    }
    int port_number = config.findPort();
    if(port_number == -1){
      throw ("Port number not provided properly");
    }

    std::shared_ptr<request_handler> echo_handler;
    echo_handler.reset(new echo_request_handler());
    std::shared_ptr<request_handler> content_handler;
    std::ifstream file_handler;
    FileReader file_reader = FileReader(file_handler);
    content_handler.reset(new content_request_handler(file_reader));
    std::shared_ptr<error_request_handler> error_handler;
    error_handler.reset(new error_request_handler());
    std::unordered_map<std::string, std::shared_ptr<request_handler>> request_handlers = {
      {ECHO_REQUEST, echo_handler},
      {SERVE_CONTENT, content_handler}
    };
    std::vector<std::shared_ptr<Servlet>> servlets = config.findPaths();
    RequestManager request_manager = RequestManager(
      request_handlers,
      error_handler,
      servlets
    );

    ServerConfig config_data(request_manager, port_number);

    server s(io_service, config_data);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
