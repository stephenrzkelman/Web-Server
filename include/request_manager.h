#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include "request_handler.h"
#include "error_request_handler.h"
#include "servlet.h"
#include <unordered_map>
#include <vector>

class RequestManager {
  std::unordered_map<std::string, std::shared_ptr<request_handler>> request_handlers_;
  std::shared_ptr<error_request_handler> error_handler_;
  std::vector<std::shared_ptr<Servlet>> servlets_;

  public:
    RequestManager(
      std::unordered_map<std::string, std::shared_ptr<request_handler>> request_handlers,
      std::shared_ptr<error_request_handler> error_handler,
      std::vector<std::shared_ptr<Servlet>> servlets
    );

    // send request to the appropriate request handler, based on which servlet it matches with.
    // then, return the response from that handler
    std::string manageRequest(boost::asio::mutable_buffer request);

    // Given a target path, pick the servlet that best matches this target
    // priority order is: 1. match type priority, 2. match length (if non-regex), 3. order placed in config
    // if no matching servlet is found, return a nullptr
    Servlet* matchTarget(std::string target_path);

    // Member function to parse HTTP request into an object
    http_message parseRequest(boost::asio::mutable_buffer request);

    // Member function to check if HTTP message object is a GET request
    bool isGetRequest(http_message request);
};

#endif // REQUEST_MANAGER_H