#ifndef _PATTERN_PARSER_H_
#define _PATTERN_PARSER_H_

#include "pattern/patternType.h"
#include "pattern.h"

class PatternParser {
private:
    static const char *get_file_path_for_pattern(const PatternType patternType) {
        switch (patternType) {
            case PatternType::COLORED_AMPLITUDE:
                return "colored_amplitude";
        }
    }

public:
    // TODO Do not parse the pattenr but a pattern blueprint to generate patterns from
    //static Pattern parse_pattern(const PatternType patternType) {
    //    return Pattern::generate_rainbow_pattern();
    //}
};

#endif