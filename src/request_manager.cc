#include "request_manager.h"
#include "registry.h"
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/trivial.hpp>
#include <string>
#include <tuple>
#include <unordered_set>

RequestManager::RequestManager(
    std::unordered_map<std::string, LocationData> &locations)
    : locations_(locations) {
  // Configure path for 404 errors
  locations_["/"] = LocationData("ErrorHandler", {});
}

http_response
RequestManager::manageRequest(boost::asio::mutable_buffer request) {
  http_request parsed_request = parseRequest(request);
  if (isGetRequest(parsed_request)) {
    BOOST_LOG_TRIVIAL(info) << "Is valid get request";
    // extract path
    std::string target_path = std::string(parsed_request.target());
    // Set payload content length
    parsed_request.prepare_payload();

    // Find longest matching prefix for location path
    std::optional<std::string> longest_matched_prefix_optional =
        matchPath(target_path);

    std::shared_ptr<RequestHandler> handler;
    if (!longest_matched_prefix_optional.has_value()) {
      BOOST_LOG_TRIVIAL(info) << "No matching path/handler found, something "
                                 "wrong on our end, returning 404 error";
      handler.reset(
          Registry::GetInstance().initializer_map_["ErrorHandler"]("", {}));
      return handler->handle_request(parsed_request);
    }

    std::string longest_matched_prefix =
        longest_matched_prefix_optional.value();
    BOOST_LOG_TRIVIAL(info) << "matched path: " << longest_matched_prefix;

    // retrieve data for location (handler name, args)
    LocationData location_data = locations_[longest_matched_prefix];

    // We have a handler for the matched location
    BOOST_LOG_TRIVIAL(info) << "Handler found: " << location_data.handler_;
    handler.reset(
        Registry::GetInstance().initializer_map_[location_data.handler_](
            longest_matched_prefix, location_data.arg_map_));
    return handler->handle_request(parsed_request);
  } else {
    // Not a valid GET request, so just echo request
    BOOST_LOG_TRIVIAL(info)
        << "Invalid get request: Proceeding to use error handler";
    std::shared_ptr<RequestHandler> handler(
        Registry::GetInstance().initializer_map_["ErrorHandler"]("", {}));
    return handler->handle_request(parsed_request);
  }
}

http_request RequestManager::parseRequest(boost::asio::mutable_buffer request) {
  boost::system::error_code error;
  boost::beast::http::request_parser<boost::beast::http::string_body> parser;
  parser.header_limit(UINT32_MAX);
  parser.put(request, error);
  if (!error && parser.is_done()) {
    return parser.get();
  } else {
    // Basic request object returned in failure
    BOOST_LOG_TRIVIAL(info) << error.message();
    return http_request();
  }
}

std::optional<std::string> RequestManager::matchPath(std::string target_path) {
  // collect all paths from config locations
  int longest_path_match = 0;
  std::string longest_prefix_matched;
  for (auto key_value : locations_) {
    std::string path_to_match = key_value.first;
    if (target_path == path_to_match ||
        target_path.starts_with(path_to_match + "/")) {
      if (path_to_match.size() > longest_path_match) {
        BOOST_LOG_TRIVIAL(info) << "new longest prefix " << path_to_match;
        longest_path_match = path_to_match.size();
        longest_prefix_matched = path_to_match;
      }
    }
  }
  if (longest_path_match > 0) {
    return longest_prefix_matched;
  } else {
    return {};
  }
}

bool RequestManager::isGetRequest(http_request request) {
  if (request.version() == 11 &&
      request.method() == boost::beast::http::verb::get) {
    return true;
  }
  return false;
}