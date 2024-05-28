#ifndef STATIC_HANDLER_H
#define STATIC_HANDLER_H

#include "registry.h"
#include "request_handler.h"
#include "filesystem/filesystem_interface.h"

const std::string STATIC_HANDLER_ROOT_ARG = "root";

class StaticHandler : public RequestHandler {
    public:
        StaticHandler(std::string path, 
                      std::unordered_map<std::string, std::string> args,
                      std::unique_ptr<FileSystemInterface> filesystem);
        http_response handle_request(const http_request& request);
        static RequestHandler* Init(std::string path, std::unordered_map<std::string, std::string> args);
        static inline ArgSet expectedArgs = {STATIC_HANDLER_ROOT_ARG};
    private:
        std::string path_;
        std::string root_;
        std::unique_ptr<FileSystemInterface> filesystem_;
};

REGISTER_HANDLER(StaticHandler);

#endif // STATIC_HANDLER_H