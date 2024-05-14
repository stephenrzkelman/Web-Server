#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include "request_handler.h"

class StaticHandler : public RequestHandler {
    public:
        StaticHandler(std::unordered_map<std::string, std::string> args);
        http_response handleRequest(const http_request& request);

    private:
        std::string root_;
};

#endif // STATIC_HANDLER_H