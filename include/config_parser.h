// An nginx config file parser.
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "servlet.h"

// string containing digits, to check if strings are numeric
const std::string DIGITS = "0123456789";

// some config keywords
// names of subcontexts
const std::string MAIN = "main";
const std::string HTTP = "http";
const std::string SERVER = "server";
const std::string LOCATION = "location";
// directives/commands
const std::string LISTEN = "listen";
const std::string BEHAVIOR = "behavior";
const std::string ROOT = "root";

// allowed directives in a given context type
const std::unordered_map<std::string, std::unordered_set<std::string>> ALLOWED_DIRECTIVES = {
  {MAIN, {}},
  {HTTP, {}},
  {SERVER, {LISTEN}},
  {LOCATION, {BEHAVIOR, ROOT}}
};
// allowed subcontexts which may appear in a given context type
const std::unordered_map<std::string, std::unordered_set<std::string>> ALLOWED_SUBCONTEXTS = {
  {MAIN, {HTTP}},
  {HTTP, {SERVER}},
  {SERVER, {LOCATION}},
  {LOCATION, {}}
};

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};

class Servlet;

// The parsed representation of the entire config.
class NginxConfig {
 public:
  NginxConfig(std::string contextName = "main");
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
  std::string contextName;
  // return child-block of an NginxConfigStatement from statements_ whose first token matches `blockName`
  // if no such NginxConfigStatement exists, or is not unique, return a nullptr
  std::vector<NginxConfig*> findChildBlocks(std::string blockName, uint argCount);
  // return NginxConfigStatement from statements_ whose first token (directive/command) matches `directiveName`
  // if no such NginxConfigStatement exists, or is not unique, return a nullptr
  // if the unique matching NginxConfigStatement has the wrong number of (argument) tokens 
  // (i.e. some number other than `argCount`) following the directive/command, return a nullptr
  // an example `directiveName` would be 'listen', which indicates which port we want the server to listen on
  std::vector<NginxConfigStatement*> findDirectives(std::string directiveName, uint argCount);
  // iterates through contexts, starting from this->contextName, to end up in main->http->server,
  // then search for valid "listen" directive and extracts port value from it.
  // return the specified port for the server.
  // if no specified port exists in the expected location (inside http{server{...}}), 
  // or the port value specified is invalid (negative, non-integer, contains letters, etc), return -1
  int findPort();
  // iterates through contexts, starting from this->contextName, to end up in main->http->server,
  // then searches for valid "location" blocks and extracts match function and desired behavior from it
  // return vector of items, each containing this matching functionality, as well as the desired behavior
  std::vector<std::unique_ptr<Servlet>> findPaths();
  // to be called from main->http->server->location context,
  // searches for valid "behavior" directive and extracts specified behavior from it
  // return the specified behavior
  // if no specified behavior exists in the expected location,
  // or is invalid (not listed in VALID_BEHAVIORS), return an empty string
  std::string findServletBehavior();
  // to be called from main->http->server->location context,
  // seraches for valid "root" directive and extracts specified root directory from it
  // return the specified root directory
  // if no specified root exists in the expected location,return an empty string
  std::string findServletRoot();
  // validate that the NginxConfig contains only allowed directives and subcontexts
  bool Validate(std::string contextType = "main");
};

// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
 public:
  NginxConfigParser() {}

  // Take a opened config file or file name (respectively) and store the
  // parsed config in the provided NginxConfig out-param.  Returns true
  // iff the input config file is valid.
  bool Parse(std::istream* config_file, NginxConfig* config);
  bool Parse(const char* file_name, NginxConfig* config);

 private:
  enum TokenType {
    TOKEN_TYPE_START = 0,
    TOKEN_TYPE_NORMAL = 1,
    TOKEN_TYPE_START_BLOCK = 2,
    TOKEN_TYPE_END_BLOCK = 3,
    TOKEN_TYPE_COMMENT = 4,
    TOKEN_TYPE_STATEMENT_END = 5,
    TOKEN_TYPE_EOF = 6,
    TOKEN_TYPE_ERROR = 7
  };
  const char* TokenTypeAsString(TokenType type);

  enum TokenParserState {
    TOKEN_STATE_INITIAL_WHITESPACE = 0,
    TOKEN_STATE_SINGLE_QUOTE = 1,
    TOKEN_STATE_DOUBLE_QUOTE = 2,
    TOKEN_STATE_TOKEN_TYPE_COMMENT = 3,
    TOKEN_STATE_TOKEN_TYPE_NORMAL = 4
  };

  TokenType ParseToken(std::istream* input, std::string* value);
};

#endif // CONFIG_PARSER_H
