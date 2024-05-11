#include "handlers/request_handler.h"

std::string RequestHandler::makeHeader(uint statusCode, std::string contentType, size_t contentLength){
    std::string responseHeader = "";
    responseHeader += HTTP_PREFIX + std::to_string(statusCode) + " " + STATUS_CODE_REASONS.at(statusCode) + CRLF;
    responseHeader += CONTENT_TYPE + contentType + CRLF;
    responseHeader += CONTENT_LENGTH + std::to_string(contentLength) + CRLF;
    responseHeader += CRLF;
    BOOST_LOG_TRIVIAL(info) << responseHeader;
    return responseHeader;
}

//Return last generated response.
std::string RequestHandler::getLastResponse() {
    return lastResponse;
}

//Return generated response header.
std::string RequestHandler::getLastResponseHeader() {
    return lastResponseHeader;
}
