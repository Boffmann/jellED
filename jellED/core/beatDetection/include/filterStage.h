#ifndef __FILTER_STAGE_JELLED_H__
#define __FILTER_STAGE_JELLED_H__

#include <stdint.h>

namespace jellED {

class FilterStage {
public:
    FilterStage(){};
    virtual ~FilterStage(){}
    virtual float apply(const float sample) = 0;
};

} // namespace jellED

#endif
