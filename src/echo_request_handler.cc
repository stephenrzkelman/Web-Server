#include "echo_request_handler.h"
#include <iostream>

//Generate a response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::vector<boost::asio::mutable_buffer> echo_request_handler::handleRequest(request_data request) {
    std::vector<boost::asio::mutable_buffer> responseBuffer;
    lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, boost::asio::buffer_size(request.raw_request));
    responseBuffer.push_back(boost::asio::buffer(lastResponseHeader));
    responseBuffer.push_back(boost::asio::buffer(request.raw_request));
    lastResponse = boost::beast::buffers_to_string(responseBuffer);
    return responseBuffer;
}