/*
	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "wiringx.h"

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
