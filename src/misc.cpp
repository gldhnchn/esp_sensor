#include "Arduino.h"
#include "misc.h"

Led::Led(uint8_t pin)
{
	pinMode(pin, OUTPUT);
	_pin = pin;
}

void Led::light_on()
{
	digitalWrite(_pin, 1);
}

void Led::light_off()
{
	digitalWrite(_pin, 0);
}

void Led::blinks(uint8_t n, uint16_t duration_in_ms)
{
	for (int i = 1; i <= n; i++)
	{
		light_on();
		delay(duration_in_ms);
		light_off();
		delay(duration_in_ms);
	}
}
