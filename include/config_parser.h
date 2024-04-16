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

// allowed directives in a given context type
const std::unordered_map<std::string, std::unordered_set<std::string>> ALLOWED_DIRECTIVES = {
  {"main", {}},
  {"http", {}},
  {"server", {"listen"}}
};
// allowed subcontexts which may appear in a given context type
const std::unordered_map<std::string, std::unordered_set<std::string>> ALLOWED_SUBCONTEXTS = {
  {"main", {"http"}},
  {"http", {"server"}},
  {"server", {""}}
};

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
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
  // return child-block of an NginxConfigStatement from statements_ whose first token matches `blockName`
  // if no such NginxConfigStatement exists, or is not unique, throw an error
  NginxConfig* findChildBlock(std::string blockName);
  // return NginxConfigStatement from statements_ whose first token (directive/command) matches `directiveName`
  // if no such NginxConfigStatement exists, or is not unique, throw an error.
  // if the unique matching NginxConfigStatement has the wrong number of (argument) tokens 
  // (i.e. some number other than `argCount`) following the directive/command, throw an error
  // an example `directiveName` would be 'listen', which indicates which port we want the server to listen on
  NginxConfigStatement* findDirective(std::string directiveName, uint argCount);
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
