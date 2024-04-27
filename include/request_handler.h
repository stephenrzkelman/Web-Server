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
    //Pure virtual function to generate a response to a given request
    virtual std::vector<boost::asio::mutable_buffer> handleRequest(boost::asio::mutable_buffer request) = 0;
    
    //Member function to return last generated response as a string.
    std::string getLastResponse();

    //(For Testing) Member function to return response header as a string.
    std::string getLastResponseHeader();
    
protected:
    //Function to formulate the response header
    std::string makeHeader(uint statusCode, std::string contentType, size_t contentLength);

    //Private member to hold last generated response header.
    std::string lastResponseHeader;

    //Private member to hold last generated response.
    std::string lastResponse;
};

#endif // REQUEST_HANDLER_H