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
    return handle_post(target, request.body());
  } else if (request.method() == boost::beast::http::verb::delete_) {
    return handle_delete(target);
  } else if (request.method() == boost::beast::http::verb::put) {
    return handle_put(target, request.body());
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
        << "CRUD[GET]: failed to read file at path " << path;
    return parseResponse(
        makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  const std::string body = body_opt.value();
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}

http_response CrudHandler::handle_post(const fs::path &path, std::string data) {
  if (!filesystem_->is_directory(path)) {
    BOOST_LOG_TRIVIAL(warning)
        << "CRUD[POST]: cannot POST to a file; path: " << path;
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }

  const std::optional<std::vector<fs::path>> files_opt =
      filesystem_->list(path);
  if (!files_opt.has_value()) {
    BOOST_LOG_TRIVIAL(debug)
        << "CRUD[POST]: failed to list files at path: " << path;
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
          << "CRUD[POST]: non-numeric filename found: " << filename
          << "; skipping";
      continue;
    }
  }

  // Increment ID, and make a new file in the path with new ID
  filesystem_->write(path / std::to_string(maxID + 1), data);
  // Create JSON containing new ID, serialize into string, and return it
  pt::ptree root, id;
  root.put("id", maxID + 1);
  std::stringstream ss;
  pt::write_json(ss, root);

  const std::string body = ss.str();
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, body.size());
  return parseResponse(header + body);
}
  
http_response CrudHandler::handle_delete(const fs::path &path) {
  if(filesystem_->is_directory(path)) {
    BOOST_LOG_TRIVIAL(warning) << "CRUD[DELETE]: request target " << path << " is not a valid path element";
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }
  
  if(!filesystem_->exists(path)) {
    // bad request error, file DNE
    return parseResponse(
        makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }

  if(!filesystem_->remove(path)){
    // internal server error, couldn't remove
    return parseResponse(
        makeHeader(INTERNAL_SERVER_ERROR_STATUS, TEXT_PLAIN, 0));
  }

  // log successful removal
  BOOST_LOG_TRIVIAL(info) << "successfully removed entity at " << path;
  return parseResponse(
        makeHeader(OK_STATUS, TEXT_PLAIN, 0));
}

http_response CrudHandler::handle_put(const fs::path &path, std::string data) {
  // check if is directory
  if (filesystem_->is_directory(path)) {
    BOOST_LOG_TRIVIAL(warning)
        << "CRUD[PUT]: cannot PUT to a directory; path: " << path;
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
  }

  // write new body    
  filesystem_->write(path, data);

  // 200 is OK as response for PUT (check RFC for detail)
  const std::string header = makeHeader(OK_STATUS, TEXT_PLAIN, 0);
  return parseResponse(header);

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
