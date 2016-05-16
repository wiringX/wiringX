#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "wiringX.h"

char *usage =
	"Usage: %s PLATTFORM GPIO CLOCK RANGE VALUE\n"
	"       GPIO to set PWM\n"
	"       CLOCK clock frequency in Hz\n"
	"       RANGE set maximum range for VALUE\n"
	"       VALUE pwm value\n"
	"Example: %s raspberrypi1b2 1 100000 256 128\n";

int main(int argc, char *argv[]) {
	char *str = NULL;
	char usagestr[160];
	int gpio = 0;
	int range = 0;
	int clock = 0;
	int value = 0;
	int i;
	bool invalid = false;

	memset(usagestr, '\0', 160);

	// expect 5 arguments
	if(argc != 6) {
		snprintf(usagestr, 159, usage, argv[0], argv[0]);
		puts(usagestr);
		return -1;
	}
	// check for a valid, numeric argument
	for(i = 2; i <= 5; i++) {
		str = argv[i];
		while(*str != '\0') {
			if(!isdigit(*str)) {
				invalid = true;
			}
			str++;
		}
		if(invalid) {
			printf("%s: Invalid GPIO %s\n", argv[0], argv[1]);
			return -1;
		}
	}

	gpio = atoi(argv[2]);
	clock = atoi(argv[3]);
	range = atoi(argv[4]);
	value = atoi(argv[5]);

	if (wiringXSetup(argv[1], NULL) != 0) {
		exit(-1);
	}

	if (pinMode(gpio, PINMODE_PWM_OUTPUT) != 0) {
		exit(-1);
	}

	if (wiringXPwmSetClock(gpio, clock) != 0) {
		exit(-1);
	}
	if (wiringXPwmSetRange(gpio, range) != 0) {
		exit(-1);
	}
	if (wiringXPwmWrite(gpio, value) != 0) {
		exit(-1);
	}
}
