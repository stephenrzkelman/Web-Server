#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <unordered_map>

const std::string HTTP_PREFIX = "HTTP/1.1 ";
const std::string CONTENT_TYPE = "Content-Type: ";
const std::string CONTENT_LENGTH = "Content-Length: ";
const std::string CRLF = "\r\n";
// Content types, see 
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
const std::string TEXT_PLAIN = "text/plain";
const std::string TEXT_HTML = "text/html";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string APPLICATION_ZIP = "application/zip";
// status codes, see
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
enum RESPONSE_CODE : unsigned int {
    OK_STATUS = 200,
    BAD_REQUEST_STATUS = 400,
    NOT_FOUND_STATUS = 404,
    INTERNAL_SERVER_ERROR_STATUS = 500
};
const std::unordered_map<uint, std::string> STATUS_CODE_REASONS = {
    {OK_STATUS, "OK"},
    {BAD_REQUEST_STATUS, "Bad Request"},
    {NOT_FOUND_STATUS, "Not Found"},
    {INTERNAL_SERVER_ERROR_STATUS, "Internal Server Error"}
};

typedef boost::beast::http::string_body http_string_body;
typedef boost::beast::http::fields http_fields;
typedef boost::beast::http::message<true, http_string_body, http_fields> http_message;

struct request_data {
    boost::asio::mutable_buffer raw_request;
    http_message* parsed_request;
    std::string root_directory;
    RESPONSE_CODE suggested_response_code;
};


class request_handler {

public:
    //Pure virtual function to generate a response to a given request
    virtual std::string handleRequest(
        request_data request
    ) = 0;
    
    //Member function to return last generated response as a string.
    std::string getLastResponse();

    //(For Testing) Member function to return response header as a string.
    std::string getLastResponseHeader();
    
protected:
    //Function to formulate the response header
    std::string makeHeader(uint statusCode, std::string contentType, size_t contentLength);

    //Private member to hold last generated response header.
    std::string lastResponseHeader;

    //Private member to hold last generated response.
    std::string lastResponse;
};

#endif // REQUEST_HANDLER_H