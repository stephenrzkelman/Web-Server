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
      last_token_type != TOKEN_TYPE_END_BLOCK) {
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
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }

  printf ("Bad transition from %s to %s\n",
          TokenTypeAsString(last_token_type),
          TokenTypeAsString(token_type));
  // in case of error, config object should remain empty
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

std::vector<NginxConfig*> NginxConfig::findChildBlocks(std::string targetBlockName, uint argCount){
  std::vector<NginxConfig*> blocks_found;
  NginxConfig* next_block;
  for (const auto& statement : statements_){
    next_block = statement->child_block_.get();
    if(next_block != nullptr){
      if(next_block->contextName == targetBlockName && 
        statement->tokens_.size() == argCount + 1) {
        blocks_found.push_back(next_block);
      }
    }
  }
  return blocks_found;
}

std::vector<NginxConfigStatement*> NginxConfig::findDirectives(std::string directiveName, uint argCount){
  std::vector<NginxConfigStatement*> directives_found;
  for (const auto& statement : statements_) {
    if (statement->tokens_[0] == directiveName &&
      statement->tokens_.size() == argCount + 1) {
      directives_found.push_back(statement.get());
    }
  }
  return directives_found;
}

bool NginxConfig::Validate(std::string baseContextType){
  std::queue<NginxConfig*> unprocessed_contexts;
  unprocessed_contexts.push(this);
  while(!unprocessed_contexts.empty()){
    NginxConfig* currentContext = unprocessed_contexts.front();
    std::string currentContextType = currentContext->contextName;
    unprocessed_contexts.pop();
    std::unordered_set<std::string> currentAllowedDirectives = ALLOWED_DIRECTIVES.at(currentContextType);
    std::unordered_set<std::string> currentAllowedSubcontexts = ALLOWED_SUBCONTEXTS.at(currentContextType);
    for(const auto& statement : currentContext->statements_){
      if(currentAllowedDirectives.find(statement->tokens_[0]) != currentAllowedDirectives.end()){
        continue;
      }
      else if(currentAllowedSubcontexts.find(statement->tokens_[0]) != currentAllowedSubcontexts.end()){
        NginxConfig* subcontext = statement->child_block_.get();
        if (subcontext == nullptr){
          return false;
        }
        unprocessed_contexts.push(subcontext);
      }
      else{
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
    possible_contexts = curConfig->findChildBlocks(HTTP, 0);
    if(possible_contexts.size() != 1){
      return -1;
    }
    curConfig = possible_contexts[0];
  }
  if(curConfig->contextName == HTTP){
    possible_contexts = curConfig->findChildBlocks(SERVER, 0);
    if (possible_contexts.size() != 1) {
      return -1;
    }
    curConfig = possible_contexts[0];
  }
  if(curConfig->contextName == SERVER){
    std::vector<NginxConfigStatement*> possible_directives = curConfig->findDirectives(LISTEN, 1);
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

std::string NginxConfig::findServletBehavior(){
  NginxConfig* curConfig = this;
  if(curConfig->contextName == LOCATION){
    std::vector<NginxConfigStatement*> possible_directives = curConfig->findDirectives(BEHAVIOR, 1);
    if (possible_directives.size() == 0) {
      // Default behavior is CONTENT
      return CONTENT;
    }
    else if(possible_directives.size() > 1){
      return "";
    }
    else { 
      NginxConfigStatement* behavior_directive = possible_directives[0];
      std::string behavior = behavior_directive->tokens_[1];
      if(VALID_BEHAVIORS.find(behavior) == VALID_BEHAVIORS.end()){
        return "";
      }
      else{
        return behavior;
      }
    }
  }
  return "";
}

std::string NginxConfig::findServletRoot(){
  NginxConfig* curConfig = this;
  if(curConfig->contextName == LOCATION){
    std::vector<NginxConfigStatement*> possible_directives = curConfig->findDirectives(ROOT, 1);
    if(possible_directives.size() != 1){
      return "";
    }
    else { 
      NginxConfigStatement* root_directive = possible_directives[0];
      std::string root = root_directive->tokens_[1];
      return root;
    }
  }
  return "";
}

std::vector<std::unique_ptr<Servlet>> NginxConfig::findPaths(){
  NginxConfig* curConfig = this;
  std::vector<NginxConfig*> possible_contexts;
  std::vector<std::unique_ptr<Servlet>> servlets;
  if(curConfig->contextName == MAIN) {
    possible_contexts = curConfig->findChildBlocks(HTTP, 0);
    if(possible_contexts.size() != 1){
      return std::vector<std::unique_ptr<Servlet>>();
    }
    curConfig = possible_contexts[0];
  }
  if(curConfig->contextName == HTTP){
    possible_contexts = curConfig->findChildBlocks(SERVER, 0);
    if (possible_contexts.size() != 1) {
      return std::vector<std::unique_ptr<Servlet>>();
    }
    curConfig = possible_contexts[0];
  }
  if(curConfig->contextName == SERVER){
    std::vector<NginxConfigStatement*> modified_location_directives = curConfig->findDirectives(LOCATION, 2);
    for (const auto& directive : modified_location_directives){
      if (MATCH_TYPES.find(directive->tokens_[1]) == MATCH_TYPES.end()){
        // return empty array if invalid match modifier string provided, to indicate error
        return std::vector<std::unique_ptr<Servlet>>();
      }
      std::string servlet_behavior = directive->child_block_.get()->findServletBehavior();
      std::string servlet_root = directive->child_block_.get()->findServletRoot();
      if ((servlet_root == "" && servlet_behavior != ECHO) || servlet_behavior == ""){
        return std::vector<std::unique_ptr<Servlet>>();
      }
      servlets.push_back(
        std::make_unique<Servlet>(Servlet(
          directive->tokens_[1],
          directive->tokens_[2],
          servlet_behavior,
          servlet_root
      )));
    }
    std::vector<NginxConfigStatement*> prefix_location_directives = curConfig->findDirectives(LOCATION, 1);
    for (const auto& directive : prefix_location_directives){
      std::string servlet_behavior = directive->child_block_.get()->findServletBehavior();
      std::string servlet_root = directive->child_block_.get()->findServletRoot();
      if ((servlet_root == "" && servlet_behavior != ECHO) || servlet_behavior == ""){
        return std::vector<std::unique_ptr<Servlet>>();
      }
      servlets.push_back(
        std::make_unique<Servlet>(Servlet(
          STANDARD_PREFIX_MATCH,
          directive->tokens_[1],
          servlet_behavior,
          servlet_root
      )));
    }
  }
  return servlets;
}