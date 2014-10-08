#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

int main(void) {
	wiringXSetup();

	pinMode(0, OUTPUT);
	pinMode(1, INPUT);
	while(1) {
		printf("Writing to pin 0: High\n");
		digitalWrite(0, HIGH);
		printf("Reading from pin 1: %d\n", digitalRead(1));
		sleep(1);
		printf("Writing to pin 0: Low\n");
		digitalWrite(0, LOW);
		printf("Reading from pin 1: %d\n", digitalRead(1));
		sleep(1);
	}
}
