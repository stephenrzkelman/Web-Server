#ifndef REGISTRY_H
#define REGISTRY_H

#include <functional>
#include <string>
#include <unordered_map>

#include "handlers/request_handler.h"

typedef std::function<RequestHandler*(std::unordered_map<std::string,std::string>)> RequestHandlerFactory;


class Registry{
    public:
        bool RegisterHandler(
            const std::string& name,
            RequestHandlerFactory factory
        ) {
            map_[name] = factory;
            return true;
        }

        static Registry& GetInstance() {
            static Registry instance;
            return instance;
        }

        std::unordered_map<std::string, RequestHandlerFactory> map_;
    private:
        Registry(){};
};

#define REGISTER_HANDLER(Goober) \
  static bool Goober##register = Registry::GetInstance().RegisterHandler(#Goober, &Goober::Init)


#endif // REGISTRY_H
