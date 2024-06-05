#include "handlers/markdown_handler.h"
#include "filesystem/filesystem.h"
#include "markdown_parser.h"
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
  } else if (request.method() == boost::beast::http::verb::put &&
                request.at(boost::beast::http::field::content_type) == MARKDOWN) {
    return handle_put(target, request.body());
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
  FILE_TYPE file_type = filesystem_->fileType(path);
  if (file_type != MARKDOWN_FILE) {
    log_handle_request_details(std::string(path), "MarkdownHandler", BAD_REQUEST_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[GET]: file requested to read is not a Markdown file " << path;
    return parseResponse(
        makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }
  const std::optional<std::string> file_opt = filesystem_->read(path);
  if (!file_opt.has_value()) {
    log_handle_request_details(std::string(path), "MarkdownHandler", BAD_REQUEST_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[GET]: failed to read file at path " << path;
    return parseResponse(
        makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }

  log_handle_request_details(std::string(path), "MarkdownHandler", OK_STATUS);
  MarkdownParser parser = MarkdownParser();
  const std::string body =  parser.parse_markdown(file_opt.value());
  const std::string header = makeHeader(OK_STATUS, TEXT_HTML, body.size());
  return parseResponse(header + body);
}

http_response MarkdownHandler::handle_put(const fs::path &path, std::string data) {
  FILE_TYPE file_type = filesystem_->fileType(path);
  if (file_type != MARKDOWN_FILE) {
    log_handle_request_details(std::string(path), "MarkdownHandler", BAD_REQUEST_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[PUT]: file requested to put is not a Markdown file " << path;
    return parseResponse(
        makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }

  // write new body
  filesystem_->write(path, data);

  // 204 no_content as response for PUT (check RFC for detail)  
  log_handle_request_details(std::string(path), "MarkdownHandler", NO_CONTENT_STATUS);
  http_response response;
  response.result(boost::beast::http::status::no_content);
  return response;
}

http_response MarkdownHandler::handle_delete(const fs::path &path) {
  // Temp response
  const std::string body = "markdown delete handled";
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}