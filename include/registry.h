#ifndef REGISTRY_H
#define REGISTRY_H

#include <functional>
#include <string>
#include <unordered_map>

#include "handlers/request_handler.h"

typedef std::function<RequestHandler*(std::string, std::unordered_map<std::string,std::string>)> RequestHandlerFactory;
typedef std::unordered_set<std::string> ArgSet;

class Registry{
    public:
        bool RegisterHandler(
            const std::string& name,
            RequestHandlerFactory factory,
            ArgSet& expected_arg_set
        ) {
            initializer_map_[name] = factory;
            expected_args_map_[name] = expected_arg_set;
            return true;
        }

        static Registry& GetInstance() {
            static Registry instance;
            return instance;
        }

        std::unordered_map<std::string, RequestHandlerFactory> initializer_map_;
        std::unordered_map<std::string, ArgSet> expected_args_map_;
    private:
        Registry(){};
};

#define REGISTER_HANDLER(HANDLER) \
  static bool HANDLER##Register = Registry::GetInstance().RegisterHandler(#HANDLER, &HANDLER::Init, HANDLER::expectedArgs)


#endif // REGISTRY_H
