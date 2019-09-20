/*
	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "../../soc/soc.h"
#include "../../wiringx.h"
#include "../platform.h"
#include "raspberrypi4.h"

struct platform_t *raspberrypi4 = NULL;

static int map[] = {
	/* 	FSEL17,	FSEL18,	FSEL27,	FSEL22 	*/
			17, 		18, 		27, 		22,
	/* 	FSEL23,	FSEL24,	FSEL25,	FSEL4 	*/
			23, 		24, 		25, 		 4,
	/* 	FSEL2,	FSEL3,	FSEL8,	FSEL7 	*/
			 2, 		 3, 		 8, 		 7,
	/*	FSEL10,	FSEL9,	FSEL11,	FSEL14	*/
			10,			 9,			11,			14,
	/*	FSEL15													*/
			15,			-1,			-1,			-1,
	/*					FSEL5,	FSEL6,	FSEL13	*/
			-1,			 5,			 6,			13,
	/*	FSEL19,	FSEL26,	FSEL12,	FSEL16	*/
			19,			26,			12,			16,
	/*	FSEL20,	FSEL21,	FSEL0,	FSEL1		*/
			20,			21,			 0,			 1
};

static int raspberrypi4ValidGPIO(int pin) {
	if(pin >= 0 && pin < (sizeof(map)/sizeof(map[0]))) {
		if(map[pin] == -1) {
			return -1;
		}
		return 0;
	} else {
		return -1;
	}
}

static int raspberrypi4Setup(void) {
	const size_t size = sizeof(map) / sizeof(map[0]);
	raspberrypi4->soc->setup();
	raspberrypi4->soc->setMap(map, size);
	raspberrypi4->soc->setIRQ(map, size);
	return 0;
}

void raspberrypi4Init(void) {
	/*
	 * The Raspberry Pi 4 uses the Broadcom 2711,
	 * but the 2711 uses the same addressen as the
	 * Broadcom 2836.
	 */
	platform_register(&raspberrypi4, "raspberrypi4");

	raspberrypi4->soc = soc_get("Broadcom", "2711");
	raspberrypi4->soc->setMap(map, sizeof(map) / sizeof(map[0]));

	raspberrypi4->digitalRead = raspberrypi4->soc->digitalRead;
	raspberrypi4->digitalWrite = raspberrypi4->soc->digitalWrite;
	raspberrypi4->pinMode = raspberrypi4->soc->pinMode;
	raspberrypi4->setup = &raspberrypi4Setup;

	raspberrypi4->isr = raspberrypi4->soc->isr;
	raspberrypi4->waitForInterrupt = raspberrypi4->soc->waitForInterrupt;

	raspberrypi4->selectableFd = raspberrypi4->soc->selectableFd;
	raspberrypi4->gc = raspberrypi4->soc->gc;

	raspberrypi4->validGPIO = &raspberrypi4ValidGPIO;
}
