#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>

class request_handler {

public:
    request_handler();

    //Member function to generate an echo response to given request
    std::vector<boost::asio::mutable_buffer> handleEchoRequest(boost::asio::mutable_buffer request);

    //Member function to parse HTTP request into an object
    boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields> parseRequest(boost::asio::mutable_buffer request);

    //Member function to check if HTTP message object is a GET request
    bool isGetRequest(boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields> request);

    //Member function to return last generated echo response as a string.
    std::string getEchoResponse();

    //(For Testing) Member function to return echo response header as a string.
    std::string getEchoResponseHeader();

    //(For Testing) Member function to return echo response header as a string.
    std::string getBadResponse();
    
private:
    //Private member to hold echo response header.
    std::string echoResponseHeader;

    //Private member to hold last generated echo response.
    std::string echoResponse;

    //Private member to hold bad generated echo response.
    std::string badResponse;

    //Private memver to check if request received is valid.
    bool isValid;
};

#endif // REQUEST_HANDLER_H