#include "application.h"
#include "DigitalOut.h"

DigitalOut::DigitalOut(uint8_t pin){
    _pin = pin;
}

void DigitalOut::init(){
    pinMode(_pin, OUTPUT);
}

void DigitalOut::on(){
    digitalWrite(_pin, HIGH);
}

void DigitalOut::off(){
    digitalWrite(_pin, LOW);
}
