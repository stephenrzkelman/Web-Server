#ifndef MARKDOWN_HANDLER_H
#define MARKDOWN_HANDLER_H

#include "filesystem/filesystem_interface.h"
#include "registry.h"
#include "request_handler.h"
#include <filesystem>
#include <memory>
#include <vector>

const std::string MARKDOWN_HANDLER_DATA_PATH_ARG = "data_path";
// specifies where the handler should get the CSS for the markdown
const std::string MARKDOWN_HANDLER_FORMAT_PATH_ARG = "format_path";

class MarkdownHandler : public RequestHandler {
public:
  MarkdownHandler(std::string path,
                  std::unordered_map<std::string, std::string> args,
                  std::unique_ptr<FileSystemInterface> filesystem);
  http_response handle_request(const http_request &request);
  static RequestHandler *
  Init(std::string path, std::unordered_map<std::string, std::string> args);
  static inline ArgSet expectedArgs = {
    MARKDOWN_HANDLER_DATA_PATH_ARG,
    MARKDOWN_HANDLER_FORMAT_PATH_ARG
  };

private:
  http_response handle_get(const std::filesystem::path &path);
  http_response handle_post(const std::filesystem::path &path,
                            std::string data);
  http_response handle_put(const std::filesystem::path &path,
                            std::string data);
  http_response handle_delete(const std::filesystem::path &path);

  // Longest matching prefix
  std::string path_;
  // Path set by data_path arg in config
  std::string data_path_;
  // Path set by format_path arg in config; the path at which to request
  // the markdown stylesheet from the static handler
  std::string format_path_;
  std::unique_ptr<FileSystemInterface> filesystem_;
};

REGISTER_HANDLER(MarkdownHandler);

#endif // MARKDOWN_HANDLER_H
