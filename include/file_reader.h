#ifndef FILE_READER_H
#define FILE_READER_H

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

const uintmax_t MB_BYTE_COUNT = 1024 * 1024;

// recognized file extensions
const std::string TEXT_FILE_EXT = ".txt";
const std::string JPG_FILE_EXT = ".jpg";
const std::string JPEG_FILE_EXT = ".jpeg";
const std::string HTML_FILE_EXT = ".html";
const std::string ZIP_FILE_EXT = ".zip";
// file type enum
enum FILE_TYPE {
    NO_MATCHING_TYPE = 0,
    TEXT_FILE = 1,
    JPG_FILE = 2,
    HTML_FILE = 3,
    ZIP_FILE = 4
};
// file type mapping
const std::unordered_map<std::string, FILE_TYPE> FILE_TYPE_MAP = {
    {TEXT_FILE_EXT, TEXT_FILE},
    {JPEG_FILE_EXT, JPG_FILE},
    {JPG_FILE_EXT, JPG_FILE},
    {HTML_FILE_EXT, HTML_FILE},
    {ZIP_FILE_EXT, ZIP_FILE}
};

class FileReader {
    public:
        FileReader(std::ifstream& file_handler);
        bool readFile(
            std::string file_name,
            std::string&  content_length
        );
        FILE_TYPE fileType(std::string file_name);
    private:
        std::ifstream& file_handler_;
};

#endif // FILE_READER_H
