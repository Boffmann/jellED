#ifndef __JELLED_RASPI_CRONO_H__
#define __JELLED_RASPI_CRONO_H__

#include "ICrono.h"

class RaspiCrono : public ICrono {
public:
    unsigned long currentTimeMicros();
};

#endif
