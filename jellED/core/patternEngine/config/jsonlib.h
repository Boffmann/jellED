#ifndef JSONLIB_H
#define JSONLIB_H

#include <string>

// remove all white space from the json string... preserving strings
std::string jsonRemoveWhiteSpace(const std::string& json);

// index a json list
std::string jsonIndexList(std::string json, int idx);

// extract a json component from json
std::string jsonExtract(const std::string& json, const std::string& name);

#endif
