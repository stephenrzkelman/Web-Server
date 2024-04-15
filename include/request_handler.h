#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <boost/asio.hpp>

class request_handler {

public:
    request_handler();

    //Member function to generate an echo response to given request
    std::vector<boost::asio::mutable_buffer> handleEchoRequest(boost::asio::mutable_buffer request);

    //Member function to return last generated echo response as a string.
    std::string getEchoResponse();
    
private:
    //Private member to hold echo response header.
    std::string echoResponseHeader;

    //Private member to hold last generated echo response.
    std::string echoResponse;
};

#endif // REQUEST_HANDLER_H