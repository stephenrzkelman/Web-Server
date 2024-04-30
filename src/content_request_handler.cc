#include "content_request_handler.h"
#include <boost/log/trivial.hpp>
#include <iostream>

content_request_handler::content_request_handler(FileReader& file_reader)
:file_reader_(file_reader){}

std::vector<boost::asio::mutable_buffer> content_request_handler::handleRequest(
  request_data request
) {
  RESPONSE_CODE status_code;
  std::string content_type;
  uintmax_t content_length = 0;
  std::vector<boost::asio::mutable_buffer> responseBuffer;
  std::string responseString;
  std::string target_file = request.root_directory + std::string(request.parsed_request->target());
  FILE_TYPE file_type = file_reader_.fileType(target_file);
  bool successful_read = file_reader_.readFile(target_file,responseString);
  if(file_type == NO_MATCHING_TYPE || !successful_read){
    status_code = NOT_FOUND_STATUS;
    content_type = TEXT_PLAIN;
  }
  else{
    status_code = OK_STATUS;
    switch(file_type){
      case TEXT_FILE:
        content_type = TEXT_PLAIN;
        break;
      case JPG_FILE:
        content_type = IMAGE_JPEG;
        break;
      case HTML_FILE:
        content_type = TEXT_HTML;
        break;
      case ZIP_FILE:
        content_type = APPLICATION_ZIP;
        break;
    }
  }
  content_length += boost::asio::buffer_size(boost::asio::buffer(responseString));
  lastResponseHeader = makeHeader(status_code, content_type, content_length);
  BOOST_LOG_TRIVIAL(info) << "saved response header";
  responseBuffer.push_back(boost::asio::buffer(lastResponseHeader));
  BOOST_LOG_TRIVIAL(info) << "added header to response";
  responseBuffer.push_back(boost::asio::buffer(responseString));
  lastResponse = boost::beast::buffers_to_string(responseBuffer);
  std::cout << lastResponse << std::endl;
  BOOST_LOG_TRIVIAL(info)<<"Sending Response";
  return responseBuffer;
}
