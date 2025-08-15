#ifndef __JELLED_RASPI_CRONO_H__
#define __JELLED_RASPI_CRONO_H__

#include "ICrono.h"

namespace jellED {

class RaspiCrono : public ICrono {
public:
    unsigned long currentTimeMicros();
};

} // namespace jellED

#endif
