#include "handlers/crud_handler.h"
#include "filesystem/filesystem.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace pt = boost::property_tree;

CrudHandler::CrudHandler(std::string path,
                         std::unordered_map<std::string, std::string> args,
                         std::unique_ptr<FileSystemInterface> filesystem)
    : path_(path), data_path_(args.at(CRUD_HANDLER_DATA_PATH_ARG)),
      filesystem_(std::move(filesystem)) {}

http_response CrudHandler::handle_request(const http_request &request) {
  BOOST_LOG_TRIVIAL(info) << "Handling CRUD request";

  // Get the "true" target path by replacing the api prefix with the actual
  // filesystem path that the handler is mounted to
  const std::string target_suffix =
      std::string(request.target()).substr(path_.size());
  const fs::path target = data_path_ + target_suffix;

  if (request.method() == boost::beast::http::verb::get) {
    return handle_get(target);
  } else if (request.method() == boost::beast::http::verb::post) {
    // TODO: Implement this
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  } else if (request.method() == boost::beast::http::verb::delete_) {
    // TODO: Implement this
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  } else if (request.method() == boost::beast::http::verb::put) {
    // TODO: Implement this
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }

  // Unimplemented functionality, return 400
  BOOST_LOG_TRIVIAL(warning) << "CRUD handler doesn't implement "
                             << request.method() << "; returning BAD_REQUEST";
  return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
}

RequestHandler *
CrudHandler::Init(std::string path,
                  std::unordered_map<std::string, std::string> args) {
  return new CrudHandler(path, args, std::make_unique<FileSystem>());
}

http_response CrudHandler::handle_get(const fs::path &path) {
  if (filesystem_->is_directory(path)) {
    return list(path);
  }

  const std::optional<std::string> body_opt = filesystem_->read(path);
  if (!body_opt.has_value()) {
    BOOST_LOG_TRIVIAL(debug)
        << "CRUD handler failed to read file at path " << path;
    return parseResponse(
        makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  const std::string body = body_opt.value();
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}

http_response CrudHandler::list(const fs::path &path) {
  std::optional<std::vector<fs::path>> file_paths_opt = filesystem_->list(path);
  if (!file_paths_opt.has_value()) {
    BOOST_LOG_TRIVIAL(debug)
        << "CRUD handler failed to list files at path " << path;
    return parseResponse(
        makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  // Add all filenames to a JSON array
  pt::ptree root, arr;
  for (const auto &file_path : file_paths_opt.value()) {
    pt::ptree element;
    element.put("", file_path.filename().string());
    arr.push_back(std::make_pair("", element));
  }
  root.add_child("files", arr);

  // Serialize the JSON array to a string, and use that as the body
  std::stringstream ss;
  pt::write_json(ss, root);
  const std::string body = ss.str();
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}
