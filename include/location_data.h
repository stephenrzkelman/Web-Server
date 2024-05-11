#ifndef LOCATION_DATA_H
#define LOCATION_DATA_H

#include <unordered_map>
#include <string>

struct LocationData {
    LocationData(std::string handler, std::unordered_map<std::string,std::string> arg_map);
    LocationData();

  // handler associated with this location
  std::string handler_;
  // args given for this location in format of <string arg, string val>
  std::unordered_map<std::string,std::string> arg_map_;

};

#endif // LOCATION_DATA_H