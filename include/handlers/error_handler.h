#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "request_handler.h"

class ErrorHandler : public RequestHandler {
    public:
        ErrorHandler(std::unordered_map<std::string, std::string> args);
        ErrorHandler();
        http_response handleRequest(const http_request& request);
};

#endif // ERROR_HANDLER_H