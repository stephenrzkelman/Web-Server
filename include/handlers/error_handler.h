#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "registry.h"
#include "request_handler.h"

class ErrorHandler : public RequestHandler {
    public:
        ErrorHandler(std::unordered_map<std::string, std::string> args);
        ErrorHandler();
        static RequestHandler* Init(std::unordered_map<std::string, std::string> args);
        http_response handleRequest(const http_request& request);
};

REGISTER_HANDLER(ErrorHandler);

#endif // ERROR_HANDLER_H