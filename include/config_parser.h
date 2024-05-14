// An nginx config file parser.
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "constants.h"
#include "location_data.h"

class NginxConfig;

// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};

// The parsed representation of the entire config.
class NginxConfig {
 public:
  NginxConfig(std::string contextName = "main");
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
  std::string contextName;
  // assumes config is valid, i.e. is a result from calling Parse() on a file
  // return vector of NginxConfigStatements found within the current context, 
  // whose first token (directive/command) matches `directiveName`
  // if no such NginxConfigStatement exists, return an empty vector
  // an example `directiveName` would be 'port', which indicates which port we want the server to listen on
  std::vector<NginxConfigStatement*> findDirectives(std::string directiveName);
  // to be called from main context
  // search for valid "port" directive and extracts port value from it.
  // return the specified port for the server.
  // if no specified port exists in the expected location (inside server{...}), 
  // or the port value specified is invalid (negative, non-integer, contains letters, etc), return -1
  int findPort();
  // to be called from main context
  // searches for "location" blocks and extracts path, handler, and root information from them
  // if succesful, returns map from path strings to LocationData
  // if any failure occurs (invalid handler specified, too many/few roots in a block, etc.),
  // it will return a nullopt.
  std::optional<std::unordered_map<std::string, LocationData>> findLocations();
  // to be called from any location context
  // searches location block for a root directive, and returns the specified path
  // if no root directive is found, returns the empty string
  // if multiple root directives are found, returns a nullopt
  // if not called from a location context, returns a nullopt
  std::optional<std::string> findRoot();
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
