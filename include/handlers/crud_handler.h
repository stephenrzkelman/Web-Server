#ifndef CRUD_HANDLER_H
#define CRUD_HANDLER_H

#include "registry.h"
#include "request_handler.h"

const std::string CRUD_HANDLER_DATA_PATH_ARG = "data_path";

class CrudHandler : public RequestHandler {
    public:
        CrudHandler(std::string path, std::unordered_map<std::string, std::string> args);
        http_response handle_request(const http_request &request);
        static RequestHandler *Init(std::string path, std::unordered_map<std::string, std::string> args);
        static inline ArgSet expectedArgs = {CRUD_HANDLER_DATA_PATH_ARG};
    private:
        std::string path_;
        std::string data_path_;
};

REGISTER_HANDLER(CrudHandler);

#endif // CRUD_HANDLER_H