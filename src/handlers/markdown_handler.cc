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

http_response MarkdownHandler::handle_post(const fs::path &path, std::string data) {
  // Temp response
  const fs::path data_path{data_path_};

  // Remove trailing slash from the path
  std::string normal_path = path.string();
  if (!normal_path.empty() && normal_path.back() == '/') {
    normal_path.pop_back();
  }
  const fs::path normal_fs_path{normal_path};

  const auto data_path_len = std::distance(data_path.begin(), data_path.end());
  const auto path_len =
      std::distance(normal_fs_path.begin(), normal_fs_path.end());
  if (!filesystem_->create_directories(path)) {
    log_handle_request_details(std::string(path), "MarkdownHandler", INTERNAL_SERVER_ERROR_STATUS);
    BOOST_LOG_TRIVIAL(warning)
        << "MARKDOWN[POST]: failing to create directories at path: " << path;
    return parseResponse(makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  // get a vector of the existing files at path 
  const std::optional<std::vector<fs::path>> files_opt =
      filesystem_->list(path);
  if (!files_opt.has_value()) {
    log_handle_request_details(std::string(path), "MarkdownHandler", INTERNAL_SERVER_ERROR_STATUS);
    BOOST_LOG_TRIVIAL(error)
        << "MARKDOWN[POST]: failed to list files at " << path << ". The directory should have been created if it did not exist, but somehow, that did not happen";
    return parseResponse(
        makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  // Get maximal ID in the path 
  int maxID = 0;
  for (const auto &filepath : files_opt.value()) {
    const std::string filename = filepath.filename().string();
    try {
      maxID = std::max(maxID, std::stoi(filename));
    } catch (const std::exception &e) {
      // Skip over non-numeric filenames
      BOOST_LOG_TRIVIAL(warning)
          << "MARKDOWN[POST]: non-numeric filename found: " << filename
          << "; skipping";
      continue;
    }
  }

  // Increment ID, and make a new file in the path with new ID
  int newID = maxID + 1;
  filesystem_->write(path / std::to_string(newID), data);

  // Return response with body indicating the id written to
  log_handle_request_details(std::string(path), "MarkdownHandler", OK_STATUS);
  const std::string body = std::to_string(newID); 
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}

http_response MarkdownHandler::handle_delete(const fs::path &path) {
  // Temp response
  const std::string body = "markdown delete handled";
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}