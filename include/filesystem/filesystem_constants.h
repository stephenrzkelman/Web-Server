#include <string>
#include <unordered_map>

// recognized file extensions
const std::string TEXT_FILE_EXT = ".txt";
const std::string JPG_FILE_EXT = ".jpg";
const std::string JPEG_FILE_EXT = ".jpeg";
const std::string PNG_FILE_EXT = ".png";
const std::string ICO_FILE_EXT = ".ico";
const std::string HTML_FILE_EXT = ".html";
const std::string ZIP_FILE_EXT = ".zip";
const std::string MARKDOWN_FILE_EXT = ".md";
const std::string CSS_FILE_EXT = ".css";
const std::string JS_FILE_EXT = ".js";
// file type enum
enum FILE_TYPE {
    NO_MATCHING_TYPE = 0,
    TEXT_FILE = 1,
    JPG_FILE = 2,
    HTML_FILE = 3,
    ZIP_FILE = 4,
    MARKDOWN_FILE = 5,
    ICO_FILE = 6,
    CSS_FILE = 7,
    JS_FILE = 8,
    PNG_FILE = 9
};
// file type mapping
const std::unordered_map<std::string, FILE_TYPE> FILE_TYPE_MAP = {
    {TEXT_FILE_EXT, TEXT_FILE},
    {JPEG_FILE_EXT, JPG_FILE},
    {JPG_FILE_EXT, JPG_FILE},
    {ICO_FILE_EXT, ICO_FILE},
    {HTML_FILE_EXT, HTML_FILE},
    {ZIP_FILE_EXT, ZIP_FILE},
    {MARKDOWN_FILE_EXT, MARKDOWN_FILE},
    {CSS_FILE_EXT, CSS_FILE},
    {JS_FILE_EXT, JS_FILE},
    {PNG_FILE_EXT, PNG_FILE}
};
