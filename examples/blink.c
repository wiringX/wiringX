#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

char *usage = "Usage: blink GPIO\n"
			"       GPIO is the GPIO to write\n"
			"Example: blink 10";

int main(int argc, char *argv[]) {
	int gpio;
	int i, n, value;
	char ch;

	// first, check for a valid, numeric argument
	// found at http://stackoverflow.com/questions/17292545
	for(i=1; i<argc; i++) {
		n = sscanf(argv[i], "%d%c", &value, &ch);
		if (n != 1) {
			printf("%s: Invalid GPIO %s\n", argv[0], argv[i]);
			return -1;
		}
	}

	// expect 1 argument for the programm blink ('blink gpio')
	// these are argv[0-1], so argc = 2
	if(argc != 2) {
		printf("%s\n", usage);
		return -1;
	}

	wiringXSetup();

	gpio = atoi(argv[1]);

	if(wiringXValidGPIO(gpio) != 0) {
		printf("%s: Invalid GPIO %d\n", argv[0], gpio);
		return -1;
	}

	pinMode(gpio, OUTPUT);
	while(1) {
		digitalWrite(gpio, HIGH);
		sleep(1);
		digitalWrite(gpio, LOW);
		sleep(1);
	}
}
