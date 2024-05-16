#include "handlers/echo_handler.h"

EchoHandler::EchoHandler(std::string path,
                         std::unordered_map<std::string, std::string> args) {}

EchoHandler::EchoHandler() {}

// Generate a response that echoes the request. Appends the request given to the
// body of a predefined response through a buffer sequence. Returns buffer
// sequence.
http_response EchoHandler::handle_request(const http_request &request) {
  // Convert the request to a string
  std::stringstream reqstream;
  reqstream << request;
  std::string requestString = reqstream.str();
  lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, requestString.size());
  lastResponse = lastResponseHeader + requestString;
  return parseResponse(lastResponse);
}

RequestHandler *
EchoHandler::Init(std::string path,
                  std::unordered_map<std::string, std::string> args) {
  return new EchoHandler(path, args);
}
