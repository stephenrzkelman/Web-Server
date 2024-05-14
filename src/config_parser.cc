// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <boost/log/trivial.hpp>
#include "config_parser.h"
#include "handler_factory.h"

NginxConfig::NginxConfig(std::string contextName)
:contextName(contextName){}

std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

std::string NginxConfigStatement::ToString(int depth) {
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i) {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr) {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i) {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  } else {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}

const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
    case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
    case TOKEN_TYPE_ERROR:         return "TOKEN_TYPE_ERROR";
    default:                       return "Unknown token type";
  }
}

NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    char prev;
    char next;
    if (!input->good()) {
      break;
    }
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE:
        switch (c) {
          case '{':
            *value = c;
            return TOKEN_TYPE_START_BLOCK;
          case '}':
            *value = c;
            return TOKEN_TYPE_END_BLOCK;
          case '#':
            *value = c;
            state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
            continue;
          case '"':
            *value = c;
            state = TOKEN_STATE_DOUBLE_QUOTE;
            continue;
          case '\'':
            *value = c;
            state = TOKEN_STATE_SINGLE_QUOTE;
            continue;
          case ';':
            *value = c;
            return TOKEN_TYPE_STATEMENT_END;
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
            *value += c;
            state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
            continue;
        }
      case TOKEN_STATE_SINGLE_QUOTE:
        prev = value->empty() ? 0 : value->back();
        next = input->peek();
        *value += c;
        if (c == '\'' && prev != '\\') {
          // End of a quoted token must be followed by whitespace or semicolon
          if(
            next != ' ' &&
            next != '\t' &&
            next != '\n' &&
            next != '\r' &&
            next != ';'
          ){
            return TOKEN_TYPE_ERROR;
          }
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE:
        prev = value->empty() ? 0 : value->back();
        next = input->peek();
        *value += c;
        if (c == '"' && prev != '\\') {
          // End of a quoted token must be followed by whitespace or semicolon
          if(
            next != ' ' &&
            next != '\t' &&
            next != '\n' &&
            next != '\r' &&
            next != ';'
          ){
            return TOKEN_TYPE_ERROR;
          }
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_TOKEN_TYPE_COMMENT:
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        *value += c;
        continue;
      case TOKEN_STATE_TOKEN_TYPE_NORMAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' ||
            c == ';' || c == '{' || c == '}') {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        }
        *value += c;
        continue;
    }
  }

  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }

  return TOKEN_TYPE_EOF;
}

bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  // Log and init parsing
  BOOST_LOG_TRIVIAL(info) << "Starting Parse"; 
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);

    // Log each token type followed by the token itself as parser loops 
    BOOST_LOG_TRIVIAL(info) << std::string(TokenTypeAsString(token_type)) + ": " + token + "\n";

    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }

    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }

    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL) {
        if (last_token_type != TOKEN_TYPE_NORMAL) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      } else {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
      NginxConfigStatement* new_block_statement = config_stack.top()->statements_.back().get();
      NginxConfig* const new_config = new NginxConfig(new_block_statement->tokens_[0]);
      new_block_statement->child_block_.reset(new_config);
      config_stack.push(new_config);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START_BLOCK) {
        // Error.
        break;
      }
      config_stack.pop();
      if (config_stack.empty()){
        break;
      }
    } else if (token_type == TOKEN_TYPE_EOF) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK) {
        // Error.
        break;
      }
      if (config_stack.size() != 1){
        break;
      }
      // validate keyword locations & arg counts
      return config->Validate();
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  BOOST_LOG_TRIVIAL(error) << "Bad transition from " << TokenTypeAsString(last_token_type) << " to " << TokenTypeAsString(token_type);
  BOOST_LOG_TRIVIAL(error) << "Parse unsuccessful";

  // parse failed
  return false;
}

bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    printf ("Failed to open config file: %s\n", file_name);
    return false;
  }

  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

std::vector<NginxConfigStatement*> NginxConfig::findDirectives(std::string directiveName){
  uint expected_arg_count = EXPECTED_ARG_COUNTS.find(directiveName)->second;
  std::vector<NginxConfigStatement*> directives_found;
  for (const auto& statement : statements_) {
    if (statement->tokens_[0] == directiveName &&
        statement->tokens_.size() == expected_arg_count + 1) {
      directives_found.push_back(statement.get());
    }
  }
  return directives_found;
}

bool NginxConfig::Validate(std::string baseContextType){
  std::queue<NginxConfig*> unprocessed_contexts;
  BOOST_LOG_TRIVIAL(info) << "Starting Validate";
  unprocessed_contexts.push(this);
  while(!unprocessed_contexts.empty()){
    NginxConfig* currentContext = unprocessed_contexts.front();
    std::string currentContextType = currentContext->contextName;
    unprocessed_contexts.pop();
    for(const auto& statement : currentContext->statements_){
      std::string statement_type = statement->tokens_[0];
      BOOST_LOG_TRIVIAL(info) << "Validating " << statement_type;
      if(VALID_DIRECTIVES.contains(statement_type) && 
        VALID_PARENT_CONTEXTS.find(statement_type)->second == currentContextType){
        NginxConfig* subcontext = statement->child_block_.get();
        if(subcontext != nullptr){
          return false;
        }
      }
      else if(VALID_CONTEXTS.contains(statement_type) && 
              VALID_PARENT_CONTEXTS.find(statement_type)->second == currentContextType){
        NginxConfig* subcontext = statement->child_block_.get();
        if (subcontext == nullptr){
          return false;
        }
        unprocessed_contexts.push(subcontext);
      }
      else{
        return false;
      }
      // check argument count of statement
      int num_args = statement->tokens_.size() - 1;
      int expected_args = EXPECTED_ARG_COUNTS.find(statement->tokens_[0])->second;
      if(num_args != expected_args){
        return false;
      }
    }
  }
  return true;
}

int NginxConfig::findPort(){
  NginxConfig* curConfig = this;
  std::vector<NginxConfig*> possible_contexts;
  if(curConfig->contextName == MAIN) {
    std::vector<NginxConfigStatement*> possible_directives = curConfig->findDirectives(PORT);
    if (possible_directives.size() != 1) {
      return -1;
    }
    else { 
      NginxConfigStatement* listen_directive = possible_directives[0];
      std::string port_arg = listen_directive->tokens_[1];
      if(port_arg.find_first_not_of(DIGITS) != std::string::npos){
        return -1;
      }
      else{
        int port_arg_num = atoi(port_arg.c_str());
        if(port_arg_num < 0 || port_arg_num > 65535){
          return -1;
        }
        else{
          return port_arg_num;
        }
      }
    }
  }
  return -1;
}

std::optional<std::string> NginxConfig::findRoot(){
  if(this->contextName != LOCATION){
    return {};
  }
  std::vector<NginxConfigStatement*> possible_directives = findDirectives(ROOT);
  if(possible_directives.size() == 0){
    return "";
  }
  else if(possible_directives.size() > 1){
    return {};
  }
  return possible_directives[0]->tokens_[1];
}

std::optional<std::unordered_map<std::string, LocationData>> NginxConfig::findLocations(){
  if(this->contextName != MAIN){
    return {}; // returns nullopt, to indicate some sort of failure
  }
  std::unordered_map<std::string, LocationData> locations;
  std::vector<NginxConfigStatement*> location_directives = this->findDirectives(LOCATION);
  for (const auto& directive : location_directives){
    std::string path = directive->tokens_[1];
    // trailing '/' should be left out of saved path, for convenience when matching
    // linux treats any number of repeated '/' as a single '/'
    while(path.back() == '/'){
      path.resize(path.size() - 1);
    }
    // no duplicate paths should be allowed
    if(locations.contains(path)){
      return {};
    }
    std::string handler = directive->tokens_[2];
    if(!VALID_HANDLERS.contains(handler)){
      return {};
    }
    LocationData location_data;
    location_data.handler_ = handler;
    NginxConfig* location_block = directive->child_block_.get();
    if(handler == STATIC_HANDLER){
      std::optional<std::string> root = location_block->findRoot();
      if(!root.has_value() || root.value() == ""){
        return {};
      }
      location_data.arg_map_.insert({ROOT, root.value()});
    }
    else if(handler == ECHO_HANDLER){
      std::optional<std::string> root = location_block->findRoot();
      // verify that no root was provided
      if(!root.has_value() || root.value() != ""){
        return {};
      }
    }
    locations.insert({path, location_data});
  }
  return locations; // may return empty map, to distinguish from failure
}
