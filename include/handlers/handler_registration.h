#ifndef HANDLER_REGISTRATION_H
#define HANDLER_REGISTRATION_H

#include "echo_handler.h"
#include "error_handler.h"
#include "static_handler.h"

// ^ Include yours above! ^

// This class is in charge of registering all handlers to the handler factory
class HandlerRegistration {
    public:
        static void register_handlers();
};

#endif // HANDLER_REGISTRATION_H