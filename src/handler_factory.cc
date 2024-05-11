#include "handler_factory.h"
#include "handlers/echo_handler.h"
#include <boost/log/trivial.hpp>

std::unordered_map<std::string,init_function> HandlerFactory::init_map_;
std::unordered_map<std::string, std::unordered_set<std::string>> HandlerFactory::arg_map_;
std::unordered_set<std::string> HandlerFactory::handlers_;

void HandlerFactory::register_handler(std::string name, init_function init, std::unordered_set<std::string> args) {
    init_map_[name] = init;
    arg_map_[name] = args;
    handlers_.insert(name);
}

RequestHandler* HandlerFactory::create(LocationData& location_data) {
    if (init_map_.find(location_data.handler_) == init_map_.end()) {
        // No handler with given name found
        return nullptr;
    }
    init_function init = init_map_[location_data.handler_];
    return init(location_data.arg_map_);
}

bool HandlerFactory::validate(std::string handler_name, std::unordered_map<std::string,std::string> args) {
    if (handlers_.find(handler_name) == handlers_.end()) {
        BOOST_LOG_TRIVIAL(info) << "Handler does not seem to be registered " << handler_name;  
        return false;
    }
    std::unordered_set<std::string> registered_args = arg_map_[handler_name];
    if (registered_args.size() != args.size()) {
        BOOST_LOG_TRIVIAL(info) << "Wrong number of arguments for" << handler_name << ". Given: " << args.size() << ". Expected: " << registered_args.size();  
        return false;
    }
    for (auto kv : args) {
        if (registered_args.find(kv.first) == registered_args.end()) {
            BOOST_LOG_TRIVIAL(info) << "Arg not found in registered args for " << handler_name << " arg:" << kv.first;  
            return false;
        }
      }
    return true;
}

RequestHandler* HandlerFactory::createEchoHandler() {
    return new EchoHandler();
}
