#ifndef DigitalOut_h
#define DigitalOut_h

#include "application.h"

class DigitalOut{
    public:
        DigitalOut(uint8_t pin);
        void on();
        void off();
        void init();
    private:
        int _pin;
};

#endif
