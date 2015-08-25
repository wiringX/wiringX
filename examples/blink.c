#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

int main(int argc, char *argv[]) {
	int pin = 0;

	wiringXSetup();

	if (argc == 1) {
		printf("blink: Using default GPIO 0\n");
	} else {
		if (argc == 2) {
			pin = atoi(argv[1]);
			if(wiringXValidGPIO(pin) != 0) {
				printf("blink: Invalid GPIO %d\n", pin);
				return -1;
			}
			printf("blink: Using GPIO %d\n", pin);
		} else {
			printf("blink: Too many arguments\n");
			return -1;
		}
	}

	pinMode(pin, OUTPUT);
	while(1) {
		digitalWrite(pin, HIGH);
		sleep(1);
		digitalWrite(pin, LOW);
		sleep(1);
	}
}
