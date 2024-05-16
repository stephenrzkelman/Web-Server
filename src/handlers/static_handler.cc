#include "handlers/static_handler.h"
#include "file_reader.h"

StaticHandler::StaticHandler(std::string path,
                             std::unordered_map<std::string, std::string> args)
    : path_(path), root_(args[STATIC_HANDLER_ROOT_ARG]) {}

http_response StaticHandler::handle_request(const http_request &request) {
  BOOST_LOG_TRIVIAL(info) << "Handling static request";
  RESPONSE_CODE status_code;
  std::string content_type;
  uintmax_t content_length = 0;
  std::string responseString;
  // if root is quoted, we need to quote the full final path
  bool quoted_path = ((root_.front() == '"' && root_.back() == '"') ||
                      (root_.front() == '\'' && root_.back() == '\''));
  std::string root_prefix = root_;
  if (quoted_path) {
    root_prefix = root_prefix.substr(1, root_.size() - 2);
  }
  std::string target_suffix =
      std::string(request.target()).substr(path_.size());
  std::string target_file = root_prefix + target_suffix;
  std::ifstream file_handler;
  FileReader file_reader = FileReader(file_handler);
  FILE_TYPE file_type = file_reader.fileType(target_file);
  if (quoted_path) {
    target_file = "'" + target_file + "'";
  }
  BOOST_LOG_TRIVIAL(info) << "File requested: " << target_file;
  bool successful_read = file_reader.readFile(target_file, responseString);
  if (file_type == NO_MATCHING_TYPE || !successful_read) {
    BOOST_LOG_TRIVIAL(error) << "No acceptable matching file type found";
    status_code = NOT_FOUND_STATUS;
    content_type = TEXT_PLAIN;
  } else {
    status_code = OK_STATUS;
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
  return new StaticHandler(path, args);
}
