#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H

#include "config_parser.h"

class ServerConfiguration{
    // this class is responsible for taking an NginxConfig object and extracting
    // the necessary information for configuring the server from it.
    public:
      ServerConfiguration(NginxConfig* config);
      uint getPort();
    private:
      uint port_;
};

#endif // SERVER_CONFIG_H
