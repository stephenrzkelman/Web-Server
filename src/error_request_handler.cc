#include "error_request_handler.h"
#include <iostream>
#include <boost/log/trivial.hpp>

//Generate a response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::string error_request_handler::handleRequest(request_data request) {
    BOOST_LOG_TRIVIAL(info) << "Handling error request";
    lastResponseHeader = makeHeader(request.suggested_response_code, TEXT_PLAIN, 0);
    lastResponse = lastResponseHeader;
    return lastResponse;
}