#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#include "wiringX.h"

char *usage =
	"Usage: %s GPIO GPIO\n"
	"       first GPIO to write to = output\n"
	"       second GPIO reacts on an interrupt = input\n"
	"Example: %s 16 20\n";

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
	char *str = NULL;
	char usagestr[180];
	int gpio_out = 0, gpio_in = 0;
	int i = 0, err = 0, invalid = 0;

	memset(usagestr, '\0', 180);

	// expect 2 arguments => argc must be 3
	if(argc != 3) {
		snprintf(usagestr, 179, usage, argv[0], argv[0]);
		printf(usagestr);
		return -1;
	}

	// check for valid, numeric arguments
	for(i=1; i<argc; i++) {
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

	gpio_out = atoi(argv[1]);
	gpio_in = atoi(argv[2]);
	if(gpio_out == gpio_in) {
		printf("%s: GPIO for output and input (interrupt) should not be the same\n", argv[0]);
		return -1;
	}


	wiringXSetup();

	if(wiringXValidGPIO(gpio_out) != 0) {
		printf("%s: Invalid GPIO %d for output\n", argv[0], gpio_out);
		return -1;
	}
	if(wiringXValidGPIO(gpio_in) != 0) {
		printf("%s: Invalid GPIO %d for input (interrupt) \n", argv[0], gpio_in);
		return -1;
	}

	pinMode(gpio_out, OUTPUT);
	if((wiringXISR(gpio_in, INT_EDGE_BOTH)) != 0) {
		return -1;
	}

	err = pthread_create(&pth, NULL, interrupt, &gpio_in);
	if(err != 0) {
		printf("Can't create thread: [%s]\n", strerror(err));
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

	printf("Main finshed, waiting for thread ...\n");
	pthread_join(pth, NULL);

	return 0;
}
