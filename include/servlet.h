#ifndef SERVLET_CONFIG_H
#define SERVLET_CONFIG_H

#include <string>
#include <tuple>
#include <unordered_set>

#include "config_parser.h"

// match type modifiers
const std::string EXACT_MATCH = "=";
const std::string PREFERRED_PREFIX_MATCH = "^~";
const std::string REGEX_CASE_SENSITIVE_MATCH = "~";
const std::string REGEX_CASE_INSENSITIVE_MATCH = "*~";
const std::string STANDARD_PREFIX_MATCH = "";
// set of match types, for checking
const std::unordered_set<std::string> MATCH_TYPES = {
  EXACT_MATCH, 
  PREFERRED_PREFIX_MATCH, 
  REGEX_CASE_SENSITIVE_MATCH,
  REGEX_CASE_INSENSITIVE_MATCH, 
  STANDARD_PREFIX_MATCH
};
// match type priorities
enum MATCH_TYPE_PRIORITY {
  EXACT_MATCH_PRIORITY = 3, 
  PREFERRED_PREFIX_MATCH_PRIORITY = 2, 
  REGEX_MATCH_PRIORITY = 1, 
  STANDARD_PREFIX_MATCH_PRIORITY = 0
};
// servlet behavior options
const std::string SERVE_CONTENT = "serve-content";
const std::string ECHO_REQUEST = "echo-request";
const std::unordered_set<std::string> VALID_BEHAVIORS = {
  SERVE_CONTENT, ECHO_REQUEST
};

class Servlet {
  public:
    Servlet(std::string match_type, std::string match_path, std::string servlet_behavior, std::string servlet_root);
    // check if a given request path matches the criteria specified for this servlet
    // return a tuple containing the priority of the match, as specified in the above enum, along with the length of the match
    // if it does not match, the length of the match returned is 0
    // (handled in controller) ties between matches are broken by priority -> match length -> order listed in config
    std::tuple<MATCH_TYPE_PRIORITY, size_t> match_path(std::string path);
    std::string servletBehavior();
    std::string servletRoot();
  private:
    std::string behavior_;
    std::string path_to_match_;
    std::string match_type_;
    std::string root_;
};

#endif // SERVLET_CONFIG_H
