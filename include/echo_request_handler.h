#ifndef ECHO_REQUEST_HANDLER_H
#define ECHO_REQUEST_HANDLER_H

#include "request_handler.h"

class echo_request_handler : public request_handler {
    public:
        std::vector<boost::asio::mutable_buffer> handleRequest(boost::asio::mutable_buffer request);
};

#endif // ECHO_REQUEST_HANDLER_H