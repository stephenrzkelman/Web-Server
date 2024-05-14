#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "request_handler.h"

class EchoHandler : public RequestHandler {
    public:
        EchoHandler(std::unordered_map<std::string, std::string> args);
        EchoHandler();
        http_response handleRequest(const http_request& request);
};

#endif // ECHO_HANDLER_H