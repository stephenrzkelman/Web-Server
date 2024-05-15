#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "registry.h"
#include "request_handler.h"

class EchoHandler : public RequestHandler {
    public:
        EchoHandler(std::unordered_map<std::string, std::string> args);
        EchoHandler();
        static RequestHandler* Init(std::unordered_map<std::string, std::string> args);
        http_response handleRequest(const http_request& request);
};

REGISTER_HANDLER(EchoHandler);

#endif // ECHO_HANDLER_H