#ifndef __JELLED_I_CRONO_H__
#define __JELLED_I_CRONO_H__

class ICrono {
public:
    virtual unsigned long currentTimeMicros() = 0;
};

#endif
