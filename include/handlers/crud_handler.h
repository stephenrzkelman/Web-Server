#ifndef CRUD_HANDLER_H
#define CRUD_HANDLER_H

#include "filesystem/filesystem_interface.h"
#include "registry.h"
#include "request_handler.h"
#include <filesystem>
#include <memory>
#include <vector>

const std::string CRUD_HANDLER_DATA_PATH_ARG = "data_path";

class CrudHandler : public RequestHandler {
public:
  CrudHandler(std::string path,
              std::unordered_map<std::string, std::string> args,
              std::unique_ptr<FileSystemInterface> filesystem);
  http_response handle_request(const http_request &request);
  static RequestHandler *
  Init(std::string path, std::unordered_map<std::string, std::string> args);
  static inline ArgSet expectedArgs = {CRUD_HANDLER_DATA_PATH_ARG};

private:
  http_response handle_get(const std::filesystem::path &path);
  http_response handle_post(const std::filesystem::path &path,
                            std::string data);
  http_response handle_delete(const std::filesystem::path &path);
  http_response handle_put(const std::filesystem::path &path,
                            std::string data);
  http_response list(const std::filesystem::path &path);

  std::string path_;
  std::string data_path_;
  std::unique_ptr<FileSystemInterface> filesystem_;
};

REGISTER_HANDLER(CrudHandler);

#endif // CRUD_HANDLER_H
