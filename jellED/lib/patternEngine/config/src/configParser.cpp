#include "configParser.h"
#include "pattern/pattern_colors.h"

#include "jsonlib.h"

#define PATTERN_TYPE_1_PARAM "p1"
#define PATTERN_TYPE_2_PARAM "p2"
#define PATTERN_TYPE_3_PARAM "p3"
#define PATTERN_TYPE_4_PARAM "p4"
#define BBP_PARAM "bpp"

#define PATTERN_COLOR_1_PARAM "c1"
#define PATTERN_COLOR_2_PARAM "c2"
#define PATTERN_COLOR_3_PARAM "c3"

PatternType toPatternType(int pt_identifier) {
    switch (pt_identifier) {
        case 1:
            return PatternType::COLORED_AMPLITUDE;
        case 2:
            return PatternType::ALTERNATING_COLORS;
        default:
            return PatternType::COLORED_AMPLITUDE;
    }
}

pattern_color toPatternColor(int int_color) {
    int red     = (int_color    & 0x00FF0000) >> 16;
    int green   = (int_color  & 0x0000FF00) >> 8;
    int blue    = (int_color   & 0x000000FF);

    Serial.println("Red in parser: ");
    Serial.println(red);

    return pattern_color{red, green, blue};
}

void JellEDConfigParser::parse_to_config(std::string& config) {
    Serial.println("Got config");
    Serial.println(config.c_str());

    String config_string = String(config.c_str());

    String json = jsonRemoveWhiteSpace(config_string);

    Serial.println("json String");
    
    Serial.println(json);

    this->pe_config.pattern1 = 
        toPatternType(jsonExtract(json, PATTERN_TYPE_1_PARAM).toInt());
    this->pe_config.pattern2 = 
        toPatternType(jsonExtract(json, PATTERN_TYPE_2_PARAM).toInt());
    this->pe_config.pattern3 = 
        toPatternType(jsonExtract(json, PATTERN_TYPE_3_PARAM).toInt());
    this->pe_config.pattern4 = 
        toPatternType(jsonExtract(json, PATTERN_TYPE_4_PARAM).toInt());
    this->pe_config.beats_per_pattern =
        jsonExtract(json, BBP_PARAM).toInt();

    this->p_config.palette_color1 =
        toPatternColor(jsonExtract(json, PATTERN_COLOR_1_PARAM).toInt());
    this->p_config.palette_color2 =
        toPatternColor(jsonExtract(json, PATTERN_COLOR_2_PARAM).toInt());
    this->p_config.palette_color3 =
        toPatternColor(jsonExtract(json, PATTERN_COLOR_3_PARAM).toInt());
}

const pattern_engine_config& JellEDConfigParser::get_pattern_engine_config() {
    return this->pe_config;
}
const pattern_config& JellEDConfigParser::get_pattern_config() {
    return this->p_config;
}
