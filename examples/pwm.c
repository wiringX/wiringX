#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

int main(void) {
	int pin = 4;

	wiringXSetup();
	pinMode(pin, PWM_OUTPUT);

	wiringXPWMEnable(pin, 0);
	wiringXSetPWM(pin, 1000, 50);
	wiringXPWMEnable(pin, 1);

	sleep(15);
	wiringXPWMEnable(pin, 0);
}
