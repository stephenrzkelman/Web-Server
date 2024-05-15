#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include "registry.h"
#include "request_handler.h"

const std::string STATIC_HANDLER_ROOT_ARG = "root";

class StaticHandler : public RequestHandler {
    public:
        StaticHandler(std::unordered_map<std::string, std::string> args);
        http_response handle_request(const http_request& request);
        static RequestHandler* Init(std::unordered_map<std::string, std::string> args);
        static inline ArgSet expectedArgs = {STATIC_HANDLER_ROOT_ARG};
    private:
        std::string root_;
};

REGISTER_HANDLER(StaticHandler);

#endif // STATIC_HANDLER_H