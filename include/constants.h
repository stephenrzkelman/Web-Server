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
// how many arguments expected for a given keyword
const std::unordered_map<std::string, uint> EXPECTED_ARG_COUNTS = {
    { LOCATION, 2 },
    { PORT, 1 },
};
// list of all keywords which specify directives
const std::unordered_set<std::string> VALID_DIRECTIVES = {
    PORT
};
// list of all keywords which specify contexts/blocks
const std::unordered_set<std::string> VALID_CONTEXTS = {
    LOCATION
};

#endif //CONSTANTS_H
