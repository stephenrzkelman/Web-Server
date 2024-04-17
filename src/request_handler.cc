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

//Parse through the request message and see if it is a valid HTTP message
boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields> request_handler::parseRequest(boost::asio::mutable_buffer request) {
    boost::system::error_code error;
    boost::beast::http::request_parser<boost::beast::http::string_body> parser;
    parser.put(request,error);
    if (!error && parser.is_done()) {
        isValid = true;
        return parser.get();
    }
    else {
        isValid = false;
        std::cerr << error.message() << std::endl;
        return parser.get();
    }
}

//Check HTTP object to see if it is a valid 1.1 GET request
bool request_handler::isGetRequest(boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields> request) {
    if (isValid && request.version() == 11 && request.method() == boost::beast::http::verb::get){
        return true;
    }
    return false;
}

//Generate an response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::vector<boost::asio::mutable_buffer> request_handler::handleEchoRequest(boost::asio::mutable_buffer request) {
    std::vector<boost::asio::mutable_buffer> responseBuffer;
    boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields> httpRequest = parseRequest(request);
    if (isGetRequest(httpRequest)) {
        lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, boost::asio::buffer_size(request));
        responseBuffer.push_back(boost::asio::buffer(lastResponseHeader));
        responseBuffer.push_back(request);
    }
    else {
        lastResponseHeader = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0);
        responseBuffer.push_back(boost::asio::buffer(lastResponseHeader));
    }
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
