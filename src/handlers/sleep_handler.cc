#include "handlers/sleep_handler.h"
#include <chrono>
#include <thread>

SleepHandler::SleepHandler(std::string path,
                           std::unordered_map<std::string, std::string> args) {}

http_response SleepHandler::handle_request(const http_request &request) {
  BOOST_LOG_TRIVIAL(info) << "Begin sleep handler";
  // Sleep for 3 seconds
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  BOOST_LOG_TRIVIAL(info) << "End sleep handler";
  std::string body = "Sleep handler test";
  lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  lastResponse = lastResponseHeader + body;
  return parseResponse(lastResponse);
}

RequestHandler *
SleepHandler::Init(std::string path,
                   std::unordered_map<std::string, std::string> args) {
  return new SleepHandler(path, args);
}
