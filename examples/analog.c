#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

int main(void) {
	int data = 0;

	if(wiringXSetup("pcduino1", NULL) == -1) {
		wiringXGC();
		return -1;
	}

	while(1) {
		data = wiringXAnalogRead(0);
		printf("result:%d\n", data);
		data = wiringXAnalogRead(1);
		printf("result:%d\n", data);
		data = wiringXAnalogRead(2);
		printf("result:%d\n", data);

		sleep(2);
	}
}
