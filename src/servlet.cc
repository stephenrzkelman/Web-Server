#include "servlet.h"
#include <regex>

Servlet::Servlet(std::string match_type, std::string match_path, std::string servlet_behavior, std::string servlet_root)
: match_type_(match_type), path_to_match_(match_path), behavior_(servlet_behavior), root_(servlet_root){}

std::tuple<MATCH_TYPE_PRIORITY, size_t> Servlet::match_path(std::string path){
    std::tuple<MATCH_TYPE_PRIORITY, size_t> match_results;
    if(match_type_ == EXACT_MATCH) {
        std::get<0>(match_results) = EXACT_MATCH_PRIORITY;
        if(path == path_to_match_){
            std::get<1>(match_results) = path.size();
        }
        else{
            std::get<1>(match_results) = 0;
        }
    }
    else if(match_type_ == PREFERRED_PREFIX_MATCH || match_type_ == STANDARD_PREFIX_MATCH) {
        if(match_type_ == PREFERRED_PREFIX_MATCH){
            std::get<0>(match_results) = PREFERRED_PREFIX_MATCH_PRIORITY;
        }
        else{
            std::get<0>(match_results) = STANDARD_PREFIX_MATCH_PRIORITY;
        }
        if(path.rfind(path_to_match_, 0) == 0){
            std::get<1>(match_results) = path_to_match_.size();
        }
        else{
            std::get<1>(match_results) = 0;
        }
    }
    else if(match_type_ == REGEX_CASE_SENSITIVE_MATCH || match_type_ == REGEX_CASE_INSENSITIVE_MATCH) {
        std::get<0>(match_results) = REGEX_MATCH_PRIORITY;
        if(match_type_ == REGEX_CASE_INSENSITIVE_MATCH){
            std::regex to_match(path_to_match_, std::regex_constants::icase);
            std::get<1>(match_results) = std::regex_search(path, to_match) ? 1 : 0;
        }
        else{
            std::regex to_match(path_to_match_);
            std::get<1>(match_results) = std::regex_search(path, to_match) ? 1 : 0;
        }
    }
    return match_results;
}

std::string Servlet::servletBehavior(){
    return behavior_;
}

std::string Servlet::servletRoot(){
    return root_;
}