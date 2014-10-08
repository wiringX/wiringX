#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

int main(void) {
	wiringXSetup();

	pinMode(0, OUTPUT);
	while(1) {
		digitalWrite(0, HIGH);
		sleep(1);
		digitalWrite(0, LOW);
		sleep(1);
	}
}
