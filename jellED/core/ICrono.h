#ifndef __JELLED_I_CRONO_H__
#define __JELLED_I_CRONO_H__

namespace jellED {

class ICrono {
public:
    virtual ~ICrono() = default;
    virtual unsigned long currentTimeMicros() = 0;
};

} // namespace jellED

#endif
