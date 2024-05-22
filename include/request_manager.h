#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include "handlers/request_handler.h"
#include "location_data.h"
#include <optional>
#include <unordered_map>
#include <vector>

class RequestManager {

  public:

    RequestManager(std::unordered_map<std::string, LocationData>& locations);

    // send request to the appropriate request handler, based on which location it matches with.
    // then, return the response from that handler
    http_response manageRequest(http_request request);

    // Find longest path in prefix which is a prefix of the target path
    // acceptable "prefix matches" are exact match (with trailing slash ignored)
    // or prefix (with trailing slash added).
    // If no such matching prefix is found, nullopt is returned
    std::optional<std::string> matchPath(std::string target_path);

    private:
      std::unordered_map<std::string, LocationData> locations_;

};

#endif // REQUEST_MANAGER_H
