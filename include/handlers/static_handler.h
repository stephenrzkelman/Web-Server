#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include "registry.h"
#include "request_handler.h"

class StaticHandler : public RequestHandler {
    public:
        StaticHandler(std::unordered_map<std::string, std::string> args);
        http_response handleRequest(const http_request& request);
        static RequestHandler* Init(std::unordered_map<std::string, std::string> args);
    private:
        std::string root_;
};

REGISTER_HANDLER(StaticHandler);

#endif // STATIC_HANDLER_H