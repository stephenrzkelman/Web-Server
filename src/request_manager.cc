#include "request_manager.h"
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <unordered_set>
#include <string>
#include <tuple>

RequestManager::RequestManager(std::unordered_map<std::string, LocationData>& locations)
  :locations_(locations) {
    //Configure path for 404 errors
    locations_["/"] = LocationData("ErrorHandler",{});
  }

http_response RequestManager::manageRequest(boost::asio::mutable_buffer request){
  http_request parsed_request = parseRequest(request);
  if(isGetRequest(parsed_request)){
    BOOST_LOG_TRIVIAL(info) << "Is valid get request";
    // extract path
    std::string target_path = std::string(parsed_request.target());

    // Find longest matching prefix for location path
    std::tuple<std::string,std::string> split_path = matchPath(target_path);
    std::string location = std::get<0>(split_path);
    std::string relative_path = std::get<1>(split_path);
    BOOST_LOG_TRIVIAL(info) << "matched path: " << location << " rel. path: " << relative_path;

    //Set payload content length
    parsed_request.prepare_payload();

    //retrieve data for location (handler name, ar  gs)
    LocationData location_data = locations_[location];

    //create specific request handler
    std::shared_ptr<RequestHandler> handler(HandlerFactory::create(location_data));
    if (handler == nullptr) {
      BOOST_LOG_TRIVIAL(info) << "No matching handler found, something wrong on our end, returning error";
      handler.reset(HandlerFactory::createErrorHandler());
      return handler->handleRequest(parsed_request);
    }
    // We have a handler for the matched location
    BOOST_LOG_TRIVIAL(info) << "Handler found: " << location_data.handler_;
    return handler->handleRequest(parsed_request);
  } else {
    // Not a valid GET request, so just echo request
    BOOST_LOG_TRIVIAL(info) << "Invalid get request: Proceeding to use error handler";
    std::shared_ptr<RequestHandler> handler(HandlerFactory::createErrorHandler());
    return handler->handleRequest(parsed_request);
  }
}

http_request RequestManager::parseRequest(boost::asio::mutable_buffer request){
  boost::system::error_code error;
  boost::beast::http::request_parser<boost::beast::http::string_body> parser;
  parser.header_limit(UINT32_MAX);
  parser.put(request,error);
  if (!error && parser.is_done()) {
    return parser.get();
  }
  else {
    //Basic request object returned in failure
    BOOST_LOG_TRIVIAL(info) << error.message();
    return http_request();
  }
}

std::tuple<std::string, std::string> RequestManager::matchPath(std::string target_path) {
  // collect all paths from config locations
  std::unordered_set<std::string> paths;
  for(auto kv : locations_) {
    paths.insert(kv.first);
  }

  int i = 0;
  for ( ; i < target_path.size(); i++) { 
    std::unordered_set<std::string> temp_paths;
    for (auto path : paths) {
      // only retain config paths that match the current target prefix
      if (target_path[i] == path[i]) {
        temp_paths.insert(path);
      }
    }

    // We have just passed the longest matching prefix
    if (temp_paths.size() == 0) {
      if (i == 0) {
        // the first character doesn't even match that of any config paths
        // set to / so error will be called
        return std::make_tuple("/", target_path);
      }
      // The next char in target string following prefix match must be "/"
      if (target_path[i] != '/') {
        // return "/" for error handler
        return std::make_tuple("/", target_path);
      }
      // The config matching path must be at the end of its string (fully matched)
      // Iterate through matching paths prior to when no more matching paths existed
      for (std::string path : paths) {
        if (path.size() == i) {
          // this is the longest matching prefix path
          std::string location = target_path.substr(0,i);
          std::string relative = target_path.substr(i,target_path.size());
          return std::make_tuple(location,relative);
        }
      }
      // return "/" for error handler
      return std::make_tuple("/", target_path);
    }

    paths = temp_paths;
  }

  // There must be an exact match to be valid (config path can't be longer than target)
  if (paths.find(target_path) != paths.end()) {
    return std::make_tuple(target_path,"");
  } else {
    return std::make_tuple("/", target_path);
  }
}

bool RequestManager::isGetRequest(http_request request){
  if (request.version() == 11 && request.method() == boost::beast::http::verb::get){
    return true;
  }
  return false;
}