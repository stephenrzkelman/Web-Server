#include "handlers/static_handler.h"
#include "filesystem/filesystem.h"

StaticHandler::StaticHandler(std::string path,
                             std::unordered_map<std::string, std::string> args,
                             std::unique_ptr<FileSystemInterface> filesystem)
    : path_(path), root_(args[STATIC_HANDLER_ROOT_ARG]), 
    filesystem_(std::move(filesystem))  {}

http_response StaticHandler::handle_request(const http_request &request) {
  BOOST_LOG_TRIVIAL(info) << "Handling static request";
  //Only respond to GET requests
  if (request.method() != boost::beast::http::verb::get){
    lastResponseHeader = makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0);
    lastResponse = lastResponseHeader;
    return parseResponse(lastResponse);
  }
  RESPONSE_CODE status_code;
  std::string content_type;
  uintmax_t content_length = 0;
  std::string responseString;
  std::string target_suffix =
      std::string(request.target()).substr(path_.size());
  const std::filesystem::path target = root_ + target_suffix;
  FILE_TYPE file_type = filesystem_->fileType(target);
  BOOST_LOG_TRIVIAL(info) << "File requested: " << target;
  std::optional<std::string> read_response = filesystem_->read(target);
  responseString = read_response.has_value() ? read_response.value() : "";
  if (file_type == NO_MATCHING_TYPE || !read_response.has_value()) {
    BOOST_LOG_TRIVIAL(warning) << "No acceptable matching file type found";
    status_code = NOT_FOUND_STATUS;
    content_type = TEXT_PLAIN;
    log_handle_request_details(std::string(request.target()), "StaticHandler", status_code);
  } else {
    status_code = OK_STATUS;
    log_handle_request_details(std::string(request.target()), "StaticHandler", status_code);
    switch (file_type) {
    case TEXT_FILE:
      BOOST_LOG_TRIVIAL(info) << "Text file requested";
      content_type = TEXT_PLAIN;
      break;
    case JPG_FILE:
      BOOST_LOG_TRIVIAL(info) << "Jpeg file requested";
      content_type = IMAGE_JPEG;
      break;
    case HTML_FILE:
      BOOST_LOG_TRIVIAL(info) << "HTML file requested";
      content_type = TEXT_HTML;
      break;
    case ZIP_FILE:
      BOOST_LOG_TRIVIAL(info) << "Zip file requested";
      content_type = APPLICATION_ZIP;
      break;
    }
  }
  content_length += responseString.size();
  lastResponseHeader = makeHeader(status_code, content_type, content_length);
  lastResponse = lastResponseHeader + responseString;
  return parseResponse(lastResponse);
}

RequestHandler *
StaticHandler::Init(std::string path,
                    std::unordered_map<std::string, std::string> args) {
  return new StaticHandler(path, args, std::make_unique<FileSystem>());
}
