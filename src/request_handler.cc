#include "request_handler.h"
#include "session.h"
#include <iostream>

std::string request_handler::makeHeader(uint statusCode, std::string contentType, size_t contentLength){
    std::string responseHeader = "";
    responseHeader += HTTP_PREFIX + std::to_string(statusCode) + " " + STATUS_CODES.at(statusCode) + CRLF;
    responseHeader += CONTENT_TYPE + contentType + CRLF;
    responseHeader += CONTENT_LENGTH + std::to_string(contentLength) + CRLF;
    responseHeader += CRLF;
    return responseHeader;
}

//Generate an response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::vector<boost::asio::mutable_buffer> request_handler::handleEchoRequest(boost::asio::mutable_buffer request) {
    std::vector<boost::asio::mutable_buffer> responseBuffer;
    lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, boost::asio::buffer_size(request));
    responseBuffer.push_back(boost::asio::buffer(lastResponseHeader));
    responseBuffer.push_back(boost::asio::buffer(request));
    lastResponse = boost::beast::buffers_to_string(responseBuffer);
    return responseBuffer;
}

//Return last generated echo response.
std::string request_handler::getLastResponse() {
    return lastResponse;
}

//Return generated echo response header.
std::string request_handler::getLastResponseHeader() {
    return lastResponseHeader;
}
