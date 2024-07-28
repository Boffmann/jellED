#ifndef __FILTER_STAGE_JELLED_H__
#define __FILTER_STAGE_JELLED_H__

#include <stdint.h>

class FilterStage {
public:
    FilterStage(){};
    virtual ~FilterStage(){}
    virtual bool apply(int16_t sample) = 0;
};

#endif