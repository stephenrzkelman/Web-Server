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
    : path_(path), 
      data_path_(args.at(MARKDOWN_HANDLER_DATA_PATH_ARG)),
      format_path_(args.at(MARKDOWN_HANDLER_FORMAT_PATH_ARG)),
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
  } else if (request.method() == boost::beast::http::verb::put &&
                request.at(boost::beast::http::field::content_type) == MARKDOWN) {
    return handle_put(target, request.body());
  } else if (request.method() == boost::beast::http::verb::delete_) {
    return handle_delete(target);
  }

  // Unimplemented functionality, return 400
  log_handle_request_details(std::string(request.target()), "MarkdownHandler", NOT_SUPPORTED_STATUS);
  BOOST_LOG_TRIVIAL(debug) << "Markdown handler doesn't implement "
                             << request.method() << "; returning NOT_SUPPORTED";
  return parseResponse(makeHeader(NOT_SUPPORTED_STATUS, TEXT_PLAIN, 0));
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
    std::string body = "Could not retrieve '" + std::string(path) + "', not a markdown file";
    std::string header = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, body.length());
    return parseResponse(header + body);
  }
  if (!filesystem_->exists(path)){
    log_handle_request_details(std::string(path), "MarkdownHandler", NOT_FOUND_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[GET]: file requested to get does not exist " << path;
    return parseResponse(makeHeader(NOT_FOUND_STATUS, TEXT_PLAIN, 0));
  }
  const std::optional<std::string> file_opt = filesystem_->read(path);
  if (!file_opt.has_value()) {
    log_handle_request_details(std::string(path), "MarkdownHandler", INTERNAL_SERVER_ERROR_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[GET]: failed to read file at path " << path;
    std::string body = "Failed to read file " + std::string(path);
    std::string header = makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, body.length());
    return parseResponse(header + body);
  }

  log_handle_request_details(std::string(path), "MarkdownHandler", OK_STATUS);
  MarkdownParser parser = MarkdownParser(format_path_);
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
    std::string body = "Failed to update '" + std::string(path) + "', not a markdown file";
    std::string header = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, body.length());
    return parseResponse(header + body);
  }
  if (!filesystem_->exists(path)) {
    log_handle_request_details(std::string(path), "MarkdownHandler", NOT_FOUND_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[PUT]: file requested to update does not exist " << path;
    return parseResponse(
        makeHeader(NOT_FOUND_STATUS, TEXT_PLAIN, 0));
  }
  // write new body
  filesystem_->write(path, data);

  // 204 no_content as response for PUT (check RFC for detail)  
  log_handle_request_details(std::string(path), "MarkdownHandler", NO_CONTENT_STATUS);
  http_response response;
  response.result(boost::beast::http::status::no_content);
  return response;
}

http_response MarkdownHandler::handle_post(const fs::path &path, std::string data) {
  FILE_TYPE file_type = filesystem_->fileType(path);
  if (file_type != MARKDOWN_FILE) {
    log_handle_request_details(std::string(path), "MarkdownHandler", BAD_REQUEST_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[POST]: file requested to post is not a Markdown file " << path;
    std::string body = "Failed to create '" + std::string(path) + "', file name must end with '.md'";
    std::string header = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, body.length());
    return parseResponse(header + body);
  }
  if (filesystem_->exists(path)) {
    log_handle_request_details(std::string(path), "MarkdownHandler", BAD_REQUEST_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[POST]: file requested to create already exists " << path;
    std::string body = "Failed to create '" + std::string(path) + "', file already exists";
    std::string header = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, body.length());
    return parseResponse(header + body);
  }
  // update body
  filesystem_->write(path, data);

  // 204 no_content as response for POST (check RFC for detail)
  log_handle_request_details(std::string(path), "MarkdownHandler", NO_CONTENT_STATUS);
  http_response response;
  response.result(boost::beast::http::status::no_content);
  return response;  
}

http_response MarkdownHandler::handle_delete(const fs::path &path) {
  // no specific file name/id given, trying to delete directory
  if (filesystem_->is_directory(path)) {
    log_handle_request_details(std::string(path), "MarkdownHandler", BAD_REQUEST_STATUS);
    BOOST_LOG_TRIVIAL(warning) << "MARKDOWN[DELETE]: request target " << path
                               << " is not a valid path element, trying to delete a directory";
    std::string header = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0);;
    std::string body = "Cannot delete '" + std::string(path) + "', not a markdown file";
    return parseResponse(header + body);
  }

  // file DNE
  if (!filesystem_->exists(path)) {
    log_handle_request_details(std::string(path), "MarkdownHandler", NOT_FOUND_STATUS);
    BOOST_LOG_TRIVIAL(debug)
        << "MARKDOWN[DELETE]: file at " << path << " does not exist";
    return parseResponse(makeHeader(NOT_FOUND_STATUS, TEXT_PLAIN, 0));
  }

  // couldn't remove
  if (!filesystem_->remove(path)) {
    log_handle_request_details(std::string(path), "MarkdownHandler", INTERNAL_SERVER_ERROR_STATUS);
    BOOST_LOG_TRIVIAL(debug)
        << "MARKDOWN[DELETE]: couldn't remove file at " << path;
    return parseResponse(
        makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  // successful removal
  log_handle_request_details(std::string(path), "MarkdownHandler", NO_CONTENT_STATUS);
  http_response response;
  response.result(boost::beast::http::status::no_content);
  return response;
}