#include "request_handler.h"
#include <boost/beast.hpp>
#include <iostream>

request_handler::request_handler(){
    echoResponseHeader = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
}

//Generate an response that echoes the request. Appends the request given to the body of a predefined response through a buffer sequence. Returns buffer sequence.
std::vector<boost::asio::mutable_buffer> request_handler::handleEchoRequest(boost::asio::mutable_buffer request){
    std::vector<boost::asio::mutable_buffer> responseBuffer;
    responseBuffer.push_back(boost::asio::buffer(echoResponseHeader));
    responseBuffer.push_back(request);
    echoResponse = boost::beast::buffers_to_string(responseBuffer);
    return responseBuffer;
}

//Return last generated echo response.
std::string request_handler::getEchoResponse(){
    return echoResponse;
}

