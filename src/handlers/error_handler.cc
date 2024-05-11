#include "handlers/error_handler.h"

ErrorHandler::ErrorHandler(std::unordered_map<std::string, std::string> args){}

//Generate a response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::string ErrorHandler::handleRequest(request_data request) {
    BOOST_LOG_TRIVIAL(info) << "Handling an error request";
    lastResponseHeader = makeHeader(404, TEXT_PLAIN, 0);
    lastResponse = lastResponseHeader;
    return lastResponse;
}