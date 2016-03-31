/*
	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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

	if(wiringXSetup("pcduino1", NULL) == -1) {
		wiringXGC();
		return -1;
	}

	if((fd = wiringXSerialOpen("/dev/ttyS0", wiringXSerial)) < 0) {
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		wiringXGC();
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


