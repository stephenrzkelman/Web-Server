#include "handlers/crud_handler.h"

CrudHandler::CrudHandler(std::string path, std::unordered_map<std::string,std::string> args)
:path_(path), data_path_(args[CRUD_HANDLER_DATA_PATH_ARG]){}

http_response CrudHandler::handle_request(const http_request& request) {
    BOOST_LOG_TRIVIAL(info) << "Handling CRUD request";

    if (request.method() == boost::beast::http::verb::get) {
        // TODO: Implement this
        return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
    } else if (request.method() == boost::beast::http::verb::post) {
        // TODO: Implement this
        return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
    } else if (request.method() == boost::beast::http::verb::delete_) {
        // TODO: Implement this
        return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
    } else if (request.method() == boost::beast::http::verb::put) {
        // TODO: Implement this
        return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
    }

    // Unimplemented functionality, return 400
    return parseResponse(makeHeader(BAD_REQUEST_STATUS, TEXT_PLAIN, 0));
}

RequestHandler* CrudHandler::Init(std::string path, std::unordered_map<std::string, std::string> args){
    return new CrudHandler(path, args);
}
