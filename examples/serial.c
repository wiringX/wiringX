#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "wiringX.h"

int main(void) {

	struct wiringXSerial_t wiringXSerial = {19200, 7, 'o', 2, 'x'};
	unsigned char data_send = 0x00;
	int fd = -1;
	int date_receive = 0;

	wiringXSetup();

	if((fd = wiringXSerialOpen("/dev/ttyS0", wiringXSerial)) < 0) {
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		return -1;
	}

	wiringXSerialPutChar(fd, data_send);

	while(1) {
		if(wiringXSerialDataAvail(fd) > 0) {
			date_receive = wiringXSerialGetChar(fd);
			printf("Data received is: %d.\n", date_receive);
		}
	}
}


