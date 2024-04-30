#include "error_request_handler.h"
#include <iostream>

//Generate a response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::vector<boost::asio::mutable_buffer> error_request_handler::handleRequest(request_data request) {
    std::vector<boost::asio::mutable_buffer> responseBuffer;
    lastResponseHeader = makeHeader(request.suggested_response_code, TEXT_PLAIN, 0);
    responseBuffer.push_back(boost::asio::buffer(lastResponseHeader));
    lastResponse = boost::beast::buffers_to_string(responseBuffer);
    return responseBuffer;
}