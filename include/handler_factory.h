#ifndef HANDLER_FACTORY_H
#define HANDLER_FACTORY_H

#include <unordered_map>
#include <unordered_set>
#include "handlers/request_handler.h"
#include "location_data.h"

typedef std::function<RequestHandler*(std::unordered_map<std::string,std::string>)> init_function;

class HandlerFactory {
  public:
    //Register handler by class name and an init function
    static void register_handler(std::string name, init_function init, std::unordered_set<std::string> args);
    //Create instance of handler with the name given
    //If such handler doesn't exist return nullptr
    static RequestHandler* create(LocationData& location_data);
    //Return boolean value indicating if handler and arg combination has been registered to the factory
    static bool validate(std::string handler_name, std::unordered_map<std::string,std::string> args);
    //Needed for when we are given request that is not a valid HTTP GET and therefore need to send a 404
    static RequestHandler* createErrorHandler();

  private:
    // Mapping from handler name to handler initialization function
    static std::unordered_map<std::string, init_function> init_map_;
    // Mapping from handler name to argument names for the handler
    static std::unordered_map<std::string, std::unordered_set<std::string>> arg_map_;
    // Set of all registered handlers
    static std::unordered_set<std::string> handlers_;
};

#endif // HANDLER_FACTORY_H
