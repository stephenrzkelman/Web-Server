#include "handlers/health_handler.h"

HealthHandler::HealthHandler(){}
HealthHandler::HealthHandler(std::string path, std::unordered_map<std::string, std::string> args) {}

http_response HealthHandler::handle_request(const http_request& request){
    log_handle_request_details(std::string(request.target()), "HealthHandler", OK_STATUS);
    const std::string okString = "Ok";
    lastResponseHeader = makeHeader(OK_STATUS, TEXT_PLAIN, okString.size());
    lastResponse = lastResponseHeader + okString;
    return parseResponse(lastResponse);
}

RequestHandler* HealthHandler::Init(std::string path, std::unordered_map<std::string, std::string> args){
    return new HealthHandler(path, args);
}

