#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

char *usage = "Usage: %s GPIO GPIO\n"
			"       first GPIO to write to = output\n"
			"       second GPIO to read from = input\n"
			"Example: %s 4 23\n";

int main(int argc, char *argv[]) {
	int gpio_out, gpio_in;
	int i, n, value;
	char ch, str [100];

	// first, check for valid, numeric arguments
	// found at http://stackoverflow.com/questions/17292545
	for(i=1; i<argc; i++) {
		n = sscanf(argv[i], "%d%c", &value, &ch);
		if (n != 1) {
			printf("%s: Invalid GPIO %s\n", argv[0], argv[i]);
			return -1;
		}
	}

	// expect 2 arguments for the programm read ('read gpio_out gpio_in')
	// these are argv[0-2], so argc = 3
	if(argc != 3) {
		sprintf(str, usage, argv[0], argv[0]);
		printf(str);
		return -1;
	}

	wiringXSetup();

	gpio_out = atoi(argv[1]);
	gpio_in = atoi(argv[2]);
	if(gpio_out == gpio_in) {
		printf("%s: GPIO for output and input should not be the same\n", argv[0]);
		return -1;
	}
	if(wiringXValidGPIO(gpio_out) != 0) {
		printf("%s: Invalid GPIO %d for output\n", argv[0], gpio_out);
		return -1;
	}
	if(wiringXValidGPIO(gpio_in) != 0) {
		printf("%s: Invalid GPIO %d for input\n", argv[0], gpio_in);
		return -1;
	}

	pinMode(gpio_out, OUTPUT);
	pinMode(gpio_in, INPUT);

	while(1) {
		printf("Writing to GPIO %d: High\n", gpio_out);
		digitalWrite(gpio_out, HIGH);
		printf("Reading from GPIO %d: %d\n", gpio_in, digitalRead(gpio_in));
		sleep(1);
		printf("Writing to GPIO %d: Low\n", gpio_out);
		digitalWrite(gpio_out, LOW);
		printf("Reading from GPIO %d: %d\n", gpio_in, digitalRead(gpio_in));
		sleep(1);
	}
}
