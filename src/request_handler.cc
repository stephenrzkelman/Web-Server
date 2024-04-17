#include "request_handler.h"
#include "session.h"
#include <iostream>

//Initialize member variable strings
request_handler::request_handler() {
    echoResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
    badResponse = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n";
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
        responseBuffer.push_back(boost::asio::buffer(echoResponseHeader));
        responseBuffer.push_back(request);
        echoResponse = boost::beast::buffers_to_string(responseBuffer);
    }
    else {
        echoResponse = badResponse;
        responseBuffer.push_back(boost::asio::buffer(badResponse));
    }
    return responseBuffer;
}

//Return last generated echo response.
std::string request_handler::getEchoResponse() {
    return echoResponse;
}

//Return generated echo response header.
std::string request_handler::getEchoResponseHeader() {
    return echoResponseHeader;
}

//Return generated bad response.
std::string request_handler::getBadResponse() {
    return badResponse;
}
