#include "server_configuration.h"

ServerConfiguration::ServerConfiguration(NginxConfig* config){
    NginxConfig* http_context = config->findChildBlock("http");
    NginxConfig* server_context = http_context->findChildBlock("server");
    NginxConfigStatement* port_directive = server_context->findDirective("listen", 1);
    port_ = (uint) atoi(port_directive->tokens_[1].c_str());
    // TODO: extract port information from config
    // TODO: TEST: port should be specified exactly once
    // TODO: TEST: port should be specified inside http{server{...}}
    // TODO: TEST: port should be an unsigned integer, within an appropriate numeric range
    // TODO: TEST: (?) no other information should be provided
}

uint ServerConfiguration::getPort(){
    return port_;
}