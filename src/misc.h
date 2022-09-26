#ifndef misc_h
#define misc_h

#include "Arduino.h"
#include <string>

class Led
{
public:
	Led(uint8_t pin);
	void light_on(),
		light_off(),
		blinks(uint8_t n = 1, uint16_t duration_in_ms = 150);

private:
	uint8_t _pin;
};

#endif
