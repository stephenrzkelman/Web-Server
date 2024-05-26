#ifndef HEALTH_HANDLER_H
#define HEALTH_HANDLER_H

#include "registry.h"
#include "request_handler.h"

class HealthHandler : public RequestHandler {
public:
    HealthHandler();
    HealthHandler(std::string path, std::unordered_map<std::string, std::string> args);
    http_response handle_request(const http_request& request);
    static RequestHandler* Init(std::string path, std::unordered_map<std::string, std::string> args);
    static inline ArgSet expectedArgs = {};
};

REGISTER_HANDLER(HealthHandler);

#endif // STATIC_HANDLER_H