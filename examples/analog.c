#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringX.h"

int main(void) {
	int data = 0;

	wiringXSetup();

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
