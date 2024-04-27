#include "request_handler.h"
#include "session.h"
#include <iostream>

std::string request_handler::makeHeader(uint statusCode, std::string contentType, size_t contentLength){
    std::string responseHeader = "";
    responseHeader += HTTP_PREFIX + std::to_string(statusCode) + " " + STATUS_CODES.at(statusCode) + CRLF;
    responseHeader += CONTENT_TYPE + contentType + CRLF;
    responseHeader += CONTENT_LENGTH + std::to_string(contentLength) + CRLF;
    responseHeader += CRLF;
    return responseHeader;
}

//Return last generated response.
std::string request_handler::getLastResponse() {
    return lastResponse;
}

//Return generated response header.
std::string request_handler::getLastResponseHeader() {
    return lastResponseHeader;
}
