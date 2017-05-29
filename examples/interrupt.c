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
#include <pthread.h>
#include <sys/syscall.h>

#include "wiringx.h"

char *usage =
	"Usage: %s platform GPIO GPIO\n"
	"       first GPIO to write to = output\n"
	"       second GPIO reacts on an interrupt = input\n"
	"Example: %s raspberrypi2 16 20\n";

void *interrupt(void *gpio_void_ptr) {
	int i = 0;
	int gpio = *(int *)gpio_void_ptr;

	while(++i < 20) {
		if(waitForInterrupt(gpio, 1000) > 0) {
			printf(">>Interrupt on GPIO %d\n", gpio);
		} else {
			printf("  Timeout on GPIO %d\n", gpio);
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	pthread_t pth;
	char *str = NULL, *platform = NULL;
	char usagestr[190];
	int gpio_out = 0, gpio_in = 0;
	int i = 0, err = 0, invalid = 0;

	memset(usagestr, '\0', 190);

	// expect 2 arguments => argc must be 3
	if(argc != 4) {
		snprintf(usagestr, 189, usage, argv[0], argv[0]);
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
		printf("%s: GPIO for output and input (interrupt) should not be the same\n", argv[0]);
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
		printf("%s: Invalid GPIO %d for input (interrupt)\n", argv[0], gpio_in);
		wiringXGC();
		return -1;
	}

	pinMode(gpio_out, PINMODE_OUTPUT);
	if((wiringXISR(gpio_in, ISR_MODE_BOTH)) != 0) {
		printf("%s: Cannot set GPIO %d to interrupt BOTH\n", argv[0], gpio_in);
		wiringXGC();
		return -1;
	}

	err = pthread_create(&pth, NULL, interrupt, &gpio_in);
	if(err != 0) {
		printf("Can't create thread: [%s]\n", strerror(err));
		wiringXGC();
		return -1;
	} else {
		printf("Thread created succesfully\n");
	}

	for(i=0; i<5; i++) {
		printf("  Writing to GPIO %d: High\n", gpio_out);
		digitalWrite(gpio_out, HIGH);
		sleep(1);
		printf("  Writing to GPIO %d: Low\n", gpio_out);
		digitalWrite(gpio_out, LOW);
		sleep(2);
	}

	printf("Main finished, waiting for thread ...\n");
	pthread_join(pth, NULL);
	wiringXGC();

	return 0;
}
