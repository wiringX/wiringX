/*
	Copyright (c) 2018

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
#include "bananapro.h"

struct platform_t *bananapro = NULL;

/* Banana Pro CON 6
 *  only IO-0 ... IO-8 are supported by this software

3v|5v
 #|5v
 #|0v
 1|#
0v|#
 0| #
 2|0v
 3| 4
3v| 5
 #|0v
 #| 6
 #|#
0v|#
 #|#
 7|0v
 #|#
 8|0v
 #|#
 #|#
0v|#

*/

/* the GPIOs IO-0 ... IO-8 (in A20 layout, see a20.c) were double checked with A20 data sheet, all others not... */
static int map[] = {
	/*	PI19,	PH2,	PI18,	PI17	*/
			167,	126,	166,	165,
	/*	PH20,	PH21,	PI16, PB3		*/
			144,	145,	164,	21,
	/*	PB13	*/
			 31
};

/*
 *  ATTENTION:
 *
 *  The intentional use of the driver does not work for the following reason:
 *  if the device is connected to a dedicated pin at CON6 of BananaPro, e.g. pin 7, then the corresponding GPIO is expected to be GPIO 1,
 *  or (according to Wiki) pin 13 is expected to correspond to GPIO 2.
 *  On the other hand, the driver uses (besides the memory mapped IO) the kernel interface /sys/class/gpio for "gpio-sunxi" platform.
 *  For Receicer devices, this kernel interface is used to read values in. But the GPIO number of that interface does NOT correspond
 *  to the expected GPIO according to the pin.
 *  Test have shown, that pin 7 at CON6 of BananaPro corresponds to gpio 4: the value can be get from /sys/class/gpio/gpio4/value
 *  Hence, a workaround (without modification of the driver) might be: use pin 7 to connect receiver, but configure GPIO 4 in pilight
 *  "433gpio": {
                        "sender": -1,
                        "receiver": 4
     }
 * This worked in case of receiver (IN direction), but probably does not work in case of sender, since sending (direction OUT) might use
 * memory mapped IO instead of  /sys/class/gpio/. But sending was not tested yet.
 * I guess that "gpio-sunxi" might not be the right kernel driver for BananaPro (basis: Bananian Linux Kernel version 3.4.113).
 *
 * */

static int bananaproValidGPIO(int pin) {
	if(pin >= 0 && pin < (sizeof(map)/sizeof(map[0]))) {
		return 0;
	} else {
		return -1;
	}
}

static int bananaproSetup(void) {
	const size_t size = sizeof(map) / sizeof(map[0]);
	bananapro->soc->setup();
	bananapro->soc->setMap(map, size);
	bananapro->soc->setIRQ(map, size);
	return 0;
}

void bananaproInit(void) {
	platform_register(&bananapro, "bananapro");

	bananapro->soc = soc_get("Allwinner", "A20");

	bananapro->digitalRead = bananapro->soc->digitalRead;
	bananapro->digitalWrite = bananapro->soc->digitalWrite;
	bananapro->pinMode = bananapro->soc->pinMode;
	bananapro->setup = &bananaproSetup;

	bananapro->isr = bananapro->soc->isr;
	bananapro->waitForInterrupt = bananapro->soc->waitForInterrupt;

	bananapro->selectableFd = bananapro->soc->selectableFd;
	bananapro->gc = bananapro->soc->gc;

	bananapro->validGPIO = &bananaproValidGPIO;
}
