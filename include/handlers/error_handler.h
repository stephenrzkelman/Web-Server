#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "registry.h"
#include "request_handler.h"

class ErrorHandler : public RequestHandler {
    public:
        ErrorHandler(std::string path, std::unordered_map<std::string, std::string> args);
        ErrorHandler();
        static RequestHandler* Init(std::string path, std::unordered_map<std::string, std::string> args);
        static inline ArgSet expectedArgs = {};
        http_response handle_request(const http_request& request);
};

REGISTER_HANDLER(ErrorHandler);

#endif // ERROR_HANDLER_H