#include "location_data.h"

LocationData::LocationData(std::string handler, std::unordered_map<std::string,std::string> arg_map)
:handler_(handler), arg_map_(arg_map){}

LocationData::LocationData(){
    handler_ = "";
    arg_map_ = std::unordered_map<std::string,std::string>();
}