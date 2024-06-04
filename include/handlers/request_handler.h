#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <unordered_map>
#include <iostream>
#include <boost/log/trivial.hpp>
#include <unordered_set>

const std::string HTTP_PREFIX = "HTTP/1.1 ";
const std::string CONTENT_TYPE = "Content-Type: ";
const std::string CONTENT_LENGTH = "Content-Length: ";
const std::string CRLF = "\r\n";
// Content types, see 
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
const std::string TEXT_PLAIN = "text/plain";
const std::string TEXT_HTML = "text/html";
const std::string IMAGE_JPEG = "image/jpeg";
const std::string IMAGE_X_ICON = "image/x-icon";
const std::string APPLICATION_ZIP = "application/zip";
const std::string JSON = "application/json";
const std::string MARKDOWN = "text/markdown";
// status codes, see
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
enum RESPONSE_CODE : unsigned int {
    OK_STATUS = 200,
    NO_CONTENT_STATUS = 204,
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
typedef boost::beast::http::field http_fields;
typedef boost::beast::http::response<http_string_body> http_response;
typedef boost::beast::http::request<http_string_body> http_request;


class RequestHandler {

public:
    //Pure virtual function to generate a response to a given request
    virtual http_response handle_request(const http_request& request) = 0;
    
    //Member function to return last generated response as a string.
    std::string getLastResponse();

    //(For Testing) Member function to return response header as a string.
    std::string getLastResponseHeader();
    
protected:
    //Function to formulate the response header
    std::string makeHeader(uint statusCode, std::string contentType, size_t contentLength);

    //Function to create http response object from given response string
    http_response parseResponse(std::string response);

    //Member function used to log the behavior of handle_request in a structured format
    void log_handle_request_details(const std::string requestTarget, std::string requestHandlerName, unsigned int responseCode);

    //Private member to hold last generated response header.
    std::string lastResponseHeader;

    //Private member to hold last generated response.
    std::string lastResponse;
    
};

#endif // REQUEST_HANDLER_H