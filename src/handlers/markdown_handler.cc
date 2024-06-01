#include "handlers/markdown_handler.h"
#include "filesystem/filesystem.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

MarkdownHandler::MarkdownHandler(std::string path,
                                 std::unordered_map<std::string, std::string> args,
                                 std::unique_ptr<FileSystemInterface> filesystem)
    : path_(path), data_path_(args.at(MARKDOWN_HANDLER_DATA_PATH_ARG)),
      filesystem_(std::move(filesystem)) {}

http_response MarkdownHandler::handle_request(const http_request &request) {
  BOOST_LOG_TRIVIAL(debug) << "Handling Markdown request";

  // Get the "true" target path by replacing the api prefix with the actual
  // filesystem path that the handler is mounted to
  const std::string target_suffix =
      std::string(request.target()).substr(path_.size());
  const fs::path target = data_path_ + target_suffix;

  if (request.method() == boost::beast::http::verb::get) {
    return handle_get(target);
  } else if (request.method() == boost::beast::http::verb::post &&
                request.at(boost::beast::http::field::content_type) == MARKDOWN) {
    return handle_post(target, request.body());
  } else if (request.method() == boost::beast::http::verb::delete_) {
    return handle_delete(target);
  }

  // Unimplemented functionality, return 400
  log_handle_request_details(std::string(request.target()), "MarkdownHandler", BAD_REQUEST_STATUS);
  BOOST_LOG_TRIVIAL(debug) << "Markdown handler doesn't implement "
                             << request.method() << "; returning BAD_REQUEST";
  return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
}

RequestHandler *
MarkdownHandler::Init(std::string path,
                  std::unordered_map<std::string, std::string> args) {
  return new MarkdownHandler(path, args, std::make_unique<FileSystem>());
}

http_response MarkdownHandler::handle_get(const fs::path &path) {
  // Temp response
  const std::string body = "markdown get handled";
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}

http_response MarkdownHandler::handle_post(const fs::path &path, std::string data) {
  // Temp response
  const std::string body = "markdown post handled";
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}

http_response MarkdownHandler::handle_delete(const fs::path &path) {
  // Temp response
  const std::string body = "markdown delete handled";
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}