#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <unordered_map>

const std::unordered_map<uint, std::string> STATUS_CODES = {
    {200, "OK"},
    {400, "Bad Request"}
};
const std::string HTTP_PREFIX = "HTTP/1.1 ";
const std::string CONTENT_TYPE = "Content-Type: ";
const std::string CONTENT_LENGTH = "Content-Length: ";
const std::string CRLF = "\r\n";
const std::string TEXT_PLAIN = "text/plain";
const uint OK_STATUS = 200;
const uint BAD_REQUEST_STATUS = 400;

class request_handler {

public:
    //Member function to generate an echo response to given request
    std::vector<boost::asio::mutable_buffer> handleEchoRequest(boost::asio::mutable_buffer request);
    
    //Member function to return last generated echo response as a string.
    std::string getLastResponse();

    //(For Testing) Member function to return echo response header as a string.
    std::string getLastResponseHeader();
    
private:
    //Function to formulate the response header
    std::string makeHeader(uint statusCode, std::string contentType, size_t contentLength);

    //Private member to hold last generated response header.
    std::string lastResponseHeader;

    //Private member to hold last generated response.
    std::string lastResponse;
};

#endif // REQUEST_HANDLER_H