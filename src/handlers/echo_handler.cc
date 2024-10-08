#include "handlers/echo_handler.h"

EchoHandler::EchoHandler(std::string path,
                         std::unordered_map<std::string, std::string> args) {}

EchoHandler::EchoHandler() {}

// Generate a response that echoes the request. Appends the request given to the
// body of a predefined response through a buffer sequence. Returns buffer
// sequence.
http_response EchoHandler::handle_request(const http_request &request) {
  //Only echo GET requests
  if (request.method() != boost::beast::http::verb::get){
    lastResponseHeader = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0);
    lastResponse = lastResponseHeader;
    return parseResponse(lastResponse);
  }
  // Convert the request to a string
  log_handle_request_details(std::string(request.target()), "EchoHandler", OK_STATUS);
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
