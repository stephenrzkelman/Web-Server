#ifndef ERROR_REQUEST_HANDLER_H
#define ERROR_REQUEST_HANDLER_H

#include "request_handler.h"

class error_request_handler : public request_handler {
    public:
        std::string handleRequest(request_data request);
};

#endif // ERROR_REQUEST_HANDLER_H