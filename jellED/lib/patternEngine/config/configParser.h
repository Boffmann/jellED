#ifndef _JELLED_CONFIG_PARSER_H__
#define _JELLED_CONFIG_PARSER_H__

#include "patternEngineConfig.h"
#include "patternConfig.h"

#include "Arduino.h"

class JellEDConfigParser {
private:
    pattern_engine_config pe_config;
    pattern_config p_config;

public:
    void parse_to_config(std::string& config);
    const pattern_engine_config& get_pattern_engine_config();
    const pattern_config& get_pattern_config();

};

#endif