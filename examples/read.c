/*
	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "wiringx.h"

char *usage =
	"Usage: %s platform GPIO GPIO\n"
	"       first GPIO to write to = output\n"
	"       second GPIO to read from = input\n"
	"Example: %s raspberrypi2 4 23\n";

int main(int argc, char *argv[]) {
	char *str = NULL, *platform = NULL;
	char usagestr[170];
	int i = 0, gpio_out = 0, gpio_in = 0, invalid = 0;

	memset(usagestr, '\0', 170);

	// expect 2 arguments => argc must be 3
	if(argc != 4) {
		snprintf(usagestr, 169, usage, argv[0], argv[0]);
		puts(usagestr);
		return -1;
	}

	// check for valid, numeric arguments
	for(i=2; i<argc; i++) {
		str = argv[i];
		while(*str != '\0') {
			if(!isdigit(*str)) {
				invalid = 1;
			}
			str++;
		}
		if(invalid == 1) {
			printf("%s: Invalid GPIO %s\n", argv[0], argv[i]);
			return -1;
		}
	}

	platform = argv[1];
	gpio_out = atoi(argv[2]);
	gpio_in = atoi(argv[3]);
	if(gpio_out == gpio_in) {
		printf("%s: GPIO for output and input should not be the same\n", argv[0]);
		return -1;
	}

	if(wiringXSetup(platform, NULL) == -1) {
		wiringXGC();
		return -1;
	}

	if(wiringXValidGPIO(gpio_out) != 0) {
		printf("%s: Invalid GPIO %d for output\n", argv[0], gpio_out);
		wiringXGC();
		return -1;
	}
	if(wiringXValidGPIO(gpio_in) != 0) {
		printf("%s: Invalid GPIO %d for input\n", argv[0], gpio_in);
		wiringXGC();
		return -1;
	}

	pinMode(gpio_out, PINMODE_OUTPUT);
	pinMode(gpio_in, PINMODE_INPUT);

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
