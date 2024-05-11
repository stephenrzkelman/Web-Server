#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include "request_handler.h"

class StaticHandler : public RequestHandler {
    public:
        StaticHandler(std::unordered_map<std::string, std::string> args);
        std::string handleRequest(request_data request);

    private:
        std::string root_;
};

#endif // STATIC_HANDLER_H