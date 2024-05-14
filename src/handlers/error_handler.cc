#include "handlers/error_handler.h"

ErrorHandler::ErrorHandler(std::unordered_map<std::string, std::string> args){}

ErrorHandler::ErrorHandler(){}

//Generate a response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
http_response ErrorHandler::handleRequest(const http_request& request) {
    BOOST_LOG_TRIVIAL(info) << "Handling an error request";
    lastResponseHeader = makeHeader(NOT_FOUND_STATUS, TEXT_PLAIN, 0);
    lastResponse = lastResponseHeader;
    return parseResponse(lastResponse);
}