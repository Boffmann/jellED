#ifndef _PATTERN_TYPE_H_
#define _PATTERN_TYPE_H_

namespace jellED {

enum class PatternType : int {
    // Shows full amplitude on beat and decreases with time since beat
    COLORED_AMPLITUDE,
    ALTERNATING_COLORS
};

namespace PatternTypeCast {
    static int to_index(PatternType patternType) {
        return (int)(patternType);
    }
}

} // namespace jellED

#endif
