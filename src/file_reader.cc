#include "file_reader.h"
#include <boost/beast.hpp>

FileReader::FileReader(std::ifstream &file_handler)
    : file_handler_(file_handler) {}

bool FileReader::readFile(std::string file_name, std::string &file_contents) {
  if (!boost::filesystem::exists(file_name)) {
    return false;
  }
  std::vector<boost::asio::mutable_buffer> returnBuffer;
  uintmax_t content_length = boost::filesystem::file_size(file_name);
  std::vector<char> file_data(content_length);
  file_handler_.open(file_name);
  file_handler_.read(file_data.data(), content_length);
  file_data.push_back('\0');
  file_contents.clear();
  returnBuffer.push_back(boost::asio::buffer(file_data, content_length));
  file_handler_.close();
  file_handler_.clear();
  file_contents = boost::beast::buffers_to_string(returnBuffer);
  return true;
}

FILE_TYPE FileReader::fileType(std::string file_name) {
  std::string file_extension =
      boost::filesystem::path(file_name).extension().string();
  if (FILE_TYPE_MAP.find(file_extension) == FILE_TYPE_MAP.end()) {
    return NO_MATCHING_TYPE;
  } else {
    return FILE_TYPE_MAP.find(file_extension)->second;
  }
}
