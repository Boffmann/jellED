#ifndef __JELLED_ESP_CRONO_H__
#define __JELLED_ESP_CRONO_H__

#include "ICrono.h"

class EspCrono : public ICrono {
public:
    virtual unsigned long currentTimeMicros();
};
#endif
