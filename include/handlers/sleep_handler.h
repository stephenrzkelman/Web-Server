#ifndef SLEEP_HANDLER_H
#define SLEEP_HANDLER_H

#include "registry.h"
#include "request_handler.h"

class SleepHandler : public RequestHandler {
    public:
        SleepHandler(std::string path, std::unordered_map<std::string, std::string> args);
        static RequestHandler* Init(std::string path, std::unordered_map<std::string, std::string> args);
        static inline ArgSet expectedArgs = {};
        http_response handle_request(const http_request& request);
};

REGISTER_HANDLER(SleepHandler);

#endif // SLEEP_HANDLER_H