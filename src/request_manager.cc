#include "request_manager.h"

#include <string>

RequestManager::RequestManager(
  std::unordered_map<std::string, std::shared_ptr<request_handler>> request_handlers,
  std::shared_ptr<error_request_handler> error_handler,
  std::vector<std::shared_ptr<Servlet>> servlets
):request_handlers_(request_handlers), error_handler_(error_handler), servlets_(servlets)
{}

std::string RequestManager::manageRequest(boost::asio::mutable_buffer request){
  http_message parsed_request = parseRequest(request);
  if(isGetRequest(parsed_request)){
    // extract path
    std::string target_path = std::string(parsed_request.target());
    // match path across servlets
    Servlet* matched_servlet = matchTarget(target_path);
    if(matched_servlet == nullptr){
      request_data data;
      data.suggested_response_code = NOT_FOUND_STATUS;
      error_handler_->handleRequest(data);
      return error_handler_->getLastResponse();
    }
    else{
      std::string servlet_behavior = matched_servlet->servletBehavior();
      if(request_handlers_.find(servlet_behavior) == request_handlers_.end()){
        request_data data;
        data.suggested_response_code = INTERNAL_SERVER_ERROR_STATUS;
        error_handler_->handleRequest(data);
        return error_handler_->getLastResponse();
      }
      else{
        request_data data;
        data.raw_request = request;
        data.parsed_request = &parsed_request;
        data.root_directory = matched_servlet->servletRoot();
        request_handlers_[servlet_behavior]->handleRequest(data);
        return request_handlers_[servlet_behavior]->getLastResponse();
      }
    }
  }
  else{
    request_data data;
    data.raw_request = request;
    data.suggested_response_code = BAD_REQUEST_STATUS;
    request_handlers_[ECHO_REQUEST]->handleRequest(data);
    return request_handlers_[ECHO_REQUEST]->getLastResponse();
  }
}

Servlet* RequestManager::matchTarget(std::string target_path){
  int highest_match_priority = -1;
  int longest_match_length = 0;
  Servlet* matched_servlet = nullptr;
  for (const auto& servlet : servlets_){
    std::tuple<int,int> match_results = servlet->match_path(target_path);
    int match_priority = std::get<0>(match_results);
    int match_length = std::get<1>(match_results);
    // higher priority
    if(match_priority > highest_match_priority && match_length > 0){
      highest_match_priority = match_priority;
      longest_match_length = match_length;
      matched_servlet = servlet.get();
    }
    // same priority, longer match
    else if(match_priority == highest_match_priority && 
            match_length > longest_match_length){
      highest_match_priority = match_priority;
      longest_match_length = match_length;
      matched_servlet = servlet.get();
    }
  }
  return matched_servlet;
}

http_message RequestManager::parseRequest(boost::asio::mutable_buffer request){
  boost::system::error_code error;
  boost::beast::http::request_parser<boost::beast::http::string_body> parser;
  parser.put(request,error);
  if (!error && parser.is_done()) {
    return parser.get();
  }
  else {
    return http_message();
  }
}

bool RequestManager::isGetRequest(http_message request){
  if (request.version() == 11 && request.method() == boost::beast::http::verb::get){
    return true;
  }
  return false;
}