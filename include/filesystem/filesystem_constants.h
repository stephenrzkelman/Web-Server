#include <string>
#include <unordered_map>

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
