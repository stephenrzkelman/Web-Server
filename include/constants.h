#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <unordered_set>
#include <unordered_map>

// misc
const std::string DIGITS = "0123456789";

// config keywords
const std::string MAIN = "main";
// directive/context keywords
const std::string LOCATION = "location";
const std::string PORT = "port";
const std::string ROOT = "root";
// how many arguments expected for a given keyword
const std::unordered_map<std::string, uint> EXPECTED_ARG_COUNTS = {
    { LOCATION, 2 },
    { PORT, 1 },
    { ROOT, 1 }
};
// in which context can a keyword appear
const std::unordered_map<std::string, std::string> VALID_PARENT_CONTEXTS = {
    { LOCATION, MAIN },
    { PORT, MAIN },
    { ROOT, LOCATION }
};
// list of all keywords which specify directives
const std::unordered_set<std::string> VALID_DIRECTIVES = {
    PORT,
    ROOT
};
// list of all keywords which specify contexts/blocks
const std::unordered_set<std::string> VALID_CONTEXTS = {
    LOCATION
};


// types of request handlers
const std::string STATIC_HANDLER = "StaticHandler";
const std::string ECHO_HANDLER = "EchoHandler";
const std::unordered_set<std::string> VALID_HANDLERS = {
    STATIC_HANDLER,
    ECHO_HANDLER
};

#endif //CONSTANTS_H
