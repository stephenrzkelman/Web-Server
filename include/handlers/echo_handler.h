#ifndef ECHO_HANDLER_H
#define ECHO_HANDLER_H

#include "request_handler.h"

class EchoHandler : public RequestHandler {
    public:
        EchoHandler(std::unordered_map<std::string, std::string> args);
        EchoHandler();
        std::string handleRequest(request_data request);
};

#endif // ECHO_HANDLER_H