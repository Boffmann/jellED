#ifndef _PATTERN_TYPE_H_
#define _PATTERN_TYPE_H_

namespace jellED {

enum class PatternType : int {
    RAINBOW
};

namespace PatternTypeCast {
    static int to_index(PatternType patternType) {
        return (int)(patternType);
    }
}

} // namespace jellED

#endif
