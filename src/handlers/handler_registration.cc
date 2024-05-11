#include "handlers/handler_registration.h"
#include "handler_factory.h"
#include <boost/log/trivial.hpp>

// Create init function for handler and register it to handler name along with its args
typedef std::unordered_map<std::string,std::string> arg_map;
#define REGISTER_HANDLER(handler,args)  HandlerFactory::register_handler(                              \
                                            #handler,                                                  \
                                            [](arg_map arg_map) { return new handler(arg_map); },             \
                                            args);                                                     \
                                        BOOST_LOG_TRIVIAL(info) << "Registering " << #handler;         \

void HandlerRegistration::register_handlers() {
    REGISTER_HANDLER(ErrorHandler, {})
    REGISTER_HANDLER(EchoHandler, {})
    REGISTER_HANDLER(StaticHandler, {"root"})

    // ^ Add yours above! Pass in your handler class name and args in {} (order doesn't matter for args) ^
    // Note there is no semicolon at the end of each line
}