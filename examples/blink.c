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
#include "../src/platform/platform.h"

char *usage =
	"Usage: %s platform GPIO\n"
	"       GPIO is the GPIO to write to\n"
	"Example: %s raspberrypi2 10\n";

int main(int argc, char *argv[]) {
	char *str = NULL, *platform = NULL;
	char usagestr[130];
	int gpio = 0, invalid = 0;

	memset(usagestr, '\0', 130);

	// expect only 1 argument => argc must be 2
	if(argc != 3) {
		snprintf(usagestr, 129, usage, argv[0], argv[0]);
		puts(usagestr);
		return -1;
	}

	// check for a valid, numeric argument
	platform = argv[1];
	str = argv[2];
	while(*str != '\0') {
		if(!isdigit(*str)) {
			invalid = 1;
		}
		str++;
	}
	if(invalid == 1) {
		printf("%s: Invalid GPIO %s\n", argv[0], argv[2]);
		return -1;
	}

	gpio = atoi(argv[2]);

	if(wiringXSetup(platform, NULL) == -1) {
		wiringXGC();
		return -1;
	}

	if(wiringXValidGPIO(gpio) != 0) {
		printf("%s: Invalid GPIO %d\n", argv[0], gpio);
		wiringXGC();
		return -1;
	}

	pinMode(gpio, PINMODE_OUTPUT);
	while(1) {
		printf("Writing to GPIO %d: High\n", gpio);
		digitalWrite(gpio, HIGH);
		sleep(1);
		printf("Writing to GPIO %d: Low\n", gpio);
		digitalWrite(gpio, LOW);
		sleep(1);
	}
}
