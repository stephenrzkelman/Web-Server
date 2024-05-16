#include "handlers/request_handler.h"

std::string RequestHandler::makeHeader(uint statusCode, std::string contentType,
                                       size_t contentLength) {
  std::string responseHeader = "";
  responseHeader += HTTP_PREFIX + std::to_string(statusCode) + " " +
                    STATUS_CODE_REASONS.at(statusCode) + CRLF;
  responseHeader += CONTENT_TYPE + contentType + CRLF;
  responseHeader += CONTENT_LENGTH + std::to_string(contentLength) + CRLF;
  responseHeader += CRLF;
  BOOST_LOG_TRIVIAL(info) << responseHeader;
  return responseHeader;
}

http_response RequestHandler::parseResponse(std::string response) {
  boost::system::error_code error;
  boost::beast::http::response_parser<boost::beast::http::string_body> parser;
  parser.body_limit(UINT32_MAX);
  const std::string constResponse = response;
  size_t bytes_parsed = 0;
  do {
    size_t position =
        parser.put(boost::asio::buffer(constResponse) + bytes_parsed, error);
    bytes_parsed += position;
    BOOST_LOG_TRIVIAL(info) << "Is parser done? :" << parser.is_done();
    BOOST_LOG_TRIVIAL(info) << "Bytes parsed: " << bytes_parsed;
    if (parser.is_done()) {
      return parser.get();
    } else if (error) {
      // Basic response object given during failure
      BOOST_LOG_TRIVIAL(info) << error.message();
      return http_response();
    }
  } while (true);
  return http_response();
}

// Return last generated response.
std::string RequestHandler::getLastResponse() { return lastResponse; }

// Return generated response header.
std::string RequestHandler::getLastResponseHeader() {
  return lastResponseHeader;
}
