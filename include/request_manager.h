#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include "handlers/request_handler.h"
#include "location_data.h"
#include <unordered_map>
#include <vector>

class RequestManager {

  public:

    RequestManager(std::unordered_map<std::string, LocationData>& locations);

    // send request to the appropriate request handler, based on which location it matches with.
    // then, return the response from that handler
    http_response manageRequest(boost::asio::mutable_buffer request);

    // split the target_path by { matched location | relative path after prefix } 
    // Match to location with longest-common-prefix
    std::tuple<std::string,std::string> matchPath(std::string target_path);

    // Member function to parse HTTP request into an object
    http_request parseRequest(boost::asio::mutable_buffer request);

    // Member function to check if HTTP message object is a GET request
    bool isGetRequest(http_request request);

    private:
      std::unordered_map<std::string, LocationData> locations_;

};

#endif // REQUEST_MANAGER_H