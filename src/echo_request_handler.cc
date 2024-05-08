#include "echo_request_handler.h"
#include <iostream>
#include <boost/log/trivial.hpp>

//Generate a response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::string echo_request_handler::handleRequest(request_data request) {
    BOOST_LOG_TRIVIAL(info) << "Handling an echo request";
    lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, boost::asio::buffer_size(request.raw_request));
    lastResponse = lastResponseHeader + boost::beast::buffers_to_string(request.raw_request);
    return lastResponse;
}