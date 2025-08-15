#include "configParser.h"
#include "pattern/pattern_colors.h"

#include "jsonlib.h"

namespace jellED {

#define PATTERN_TYPE_1_PARAM "p1"
#define PATTERN_TYPE_2_PARAM "p2"
#define PATTERN_TYPE_3_PARAM "p3"
#define PATTERN_TYPE_4_PARAM "p4"
#define BBP_PARAM "bpp"

#define PATTERN_COLOR_1_PARAM "c1"
#define PATTERN_COLOR_2_PARAM "c2"
#define PATTERN_COLOR_3_PARAM "c3"

JellEDConfigParser::JellEDConfigParser(IPlatformUtils& platformUtils) 
    : platformUtils(platformUtils)
{}

PatternType toPatternType(int pt_identifier){
    switch (pt_identifier) {
        case 1:
            return PatternType::COLORED_AMPLITUDE;
        case 2:
            return PatternType::ALTERNATING_COLORS;
        default:
            return PatternType::COLORED_AMPLITUDE;
    }
}

pattern_color toPatternColor(int int_color, IPlatformUtils& platformUtils) {
    int red     = (int_color    & 0x00FF0000) >> 16;
    int green   = (int_color  & 0x0000FF00) >> 8;
    int blue    = (int_color   & 0x000000FF);

    platformUtils.logger().log("Red in parser: ");
    platformUtils.logger().log(std::to_string(red));

    return pattern_color{red, green, blue};
}

void JellEDConfigParser::parse_to_config(std::string& config) {
    platformUtils.logger().log("Got config");
    platformUtils.logger().log(config);

    std::string config_string = std::string(config.c_str());

    std::string json = jsonRemoveWhiteSpace(config_string);

    platformUtils.logger().log("json String");
    platformUtils.logger().log(json);

    this->pe_config.pattern1 = 
        toPatternType(std::stoi(jsonExtract(json, PATTERN_TYPE_1_PARAM)));
    this->pe_config.pattern2 = 
        toPatternType(std::stoi(jsonExtract(json, PATTERN_TYPE_2_PARAM)));
    this->pe_config.pattern3 = 
        toPatternType(std::stoi(jsonExtract(json, PATTERN_TYPE_3_PARAM)));
    this->pe_config.pattern4 = 
        toPatternType(std::stoi(jsonExtract(json, PATTERN_TYPE_4_PARAM)));
    this->pe_config.beats_per_pattern =
        std::stoi(jsonExtract(json, BBP_PARAM));

    this->p_config.palette_color1 =
        toPatternColor(std::stoi(jsonExtract(json, PATTERN_COLOR_1_PARAM)), this->platformUtils);
    this->p_config.palette_color2 =
        toPatternColor(std::stoi(jsonExtract(json, PATTERN_COLOR_2_PARAM)), this->platformUtils);
    this->p_config.palette_color3 =
        toPatternColor(std::stoi(jsonExtract(json, PATTERN_COLOR_3_PARAM)), this->platformUtils);
}

const pattern_engine_config& JellEDConfigParser::get_pattern_engine_config() {
    return this->pe_config;
}
const pattern_config& JellEDConfigParser::get_pattern_config() {
    return this->p_config;
}

} // end namespace jellED
