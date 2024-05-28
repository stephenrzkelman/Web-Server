#include "handlers/error_handler.h"

ErrorHandler::ErrorHandler(std::string path,
                           std::unordered_map<std::string, std::string> args) {}

ErrorHandler::ErrorHandler() {}

// Generate a response that echoes the request. Appends the request given to the
// body of a predefined response through a buffer sequence. Returns buffer
// sequence.
http_response ErrorHandler::handle_request(const http_request &request) {
  log_handle_request_details(std::string(request.target()), "ErrorHandler", NOT_FOUND_STATUS);
  lastResponseHeader = makeHeader(NOT_FOUND_STATUS, TEXT_PLAIN, 0);
  lastResponse = lastResponseHeader;
  return parseResponse(lastResponse);
}

RequestHandler *
ErrorHandler::Init(std::string path,
                   std::unordered_map<std::string, std::string> args) {
  return new ErrorHandler(path, args);
}
