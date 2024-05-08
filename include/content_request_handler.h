#ifndef CONTENT_REQUEST_HANDLER_H
#define CONTENT_REQUEST_HANDLER_H

#include "file_reader.h"
#include "request_handler.h"
#include <boost/filesystem.hpp>
#include <fstream>

class content_request_handler : public request_handler {
    public:
        content_request_handler(FileReader& file_reader);

        std::string handleRequest(
            request_data request
        );

    private:
        FileReader file_reader_;
};

#endif // CONTENT_REQUEST_HANDLER_H