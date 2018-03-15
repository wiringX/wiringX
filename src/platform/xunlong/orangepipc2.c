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
#include "orangepipc2.h"

struct platform_t *orangepipc2 = NULL;

/*
3v|5v
 8|5v
 9|0v
 7|15
0v|16
 0| 1
 2|0v
 3| 4
3v| 5
12|0v
13| 6
14|10
0v|11
20|19
21|0v
22|26
23|0v
24|27
25|28
0v|29

0v
17: PA5
18: PA4

LED RED: PA20
LED GREEN: ?
*/

static int irq[] = {
 /*  0,   1,   2,   3 */
     1,  -1,   0,   3,
 /*  4,   5,   6,   7 */
    -1,  -1,   2,   6,
 /*  8,   9,  10,  11 */
	  12,  11,  -1,  21,
 /* 12,  13,  14,  15 */
    -1,  -1,  14,  -1,
 /* 16,  17,  18,  19 */
    -1,   5,   4,  -1,
 /* 20,  21,  22,  23 */
    -1,  -1,   8,   9,
 /* 24,  25,  26,  27 */
    10,  -1, 200, 201,
 /* 28,  29 */
   198, 199
};

static int map[] = {
 /*  PA1, PD14,  PA0,  PA3 */
       1,   53,    0,    3,
 /*  PC4,  PC7,  PA2,  PA6 */
      26,   29,    2,    6,
 /* PA12, PA11, PA13, PA21 */
      12,   11,   13,   21,
 /* PA15, PA16, PA14,  PC5 */
      15,   16,   14,   27,
 /*  PC6,  PA5,  PA4, PA18 */
      28,    5,    4,   18,
 /* PA19, PA7,  PA8,   PA9 */
      19,    7,   8,     9,
 /* PA10, PD11, PG8,   PG9 */
      10,   50,  88,    89,
 /*  PG6,  PG7, PA20 (Red LED) */
      86,   87,   20
};

static int orangepipc2ValidGPIO(int pin) {
	if(pin >= 0 && pin < (sizeof(map)/sizeof(map[0]))) {
		if(map[pin] == -1) {
			return -1;
		}
		return 0;
	} else {
		return -1;
	}
	return 0;
}

static int orangepipc2PinMode(int i, enum pinmode_t mode) {
	if(map[i] == -1) {
		return -1;
	}
	return orangepipc2->soc->pinMode(i, mode);
}

static int orangepipc2DigitalWrite(int i, enum digital_value_t value) {
	if(map[i] == -1) {
		return -1;
	}
	return orangepipc2->soc->digitalWrite(i, value);
}

static int orangepipc2DigitalRead(int i) {
	/* Red LED */
	if(i == 30) {
		return -1;
	}
	return orangepipc2->soc->digitalRead(i);
}

static int orangepipc2Setup(void) {
	const size_t msize = sizeof(map) / sizeof(map[0]);
	const size_t qsize = sizeof(irq) / sizeof(irq[0]);
	orangepipc2->soc->setup();
	orangepipc2->soc->setMap(map, msize);
	orangepipc2->soc->setIRQ(irq, qsize);
	return 0;
}

static int orangepipc2ISR(int i, enum isr_mode_t mode) {
	if(irq[i] == -1) {
		return -1;
	}
	orangepipc2->soc->isr(i, mode);
	return 0;
}

void orangepipc2Init(void) {
	platform_register(&orangepipc2, "orangepipc2");

	orangepipc2->soc = soc_get("Allwinner", "H5");

	orangepipc2->digitalRead = &orangepipc2DigitalRead;
	orangepipc2->digitalWrite = &orangepipc2DigitalWrite;
	orangepipc2->pinMode = &orangepipc2PinMode;
	orangepipc2->setup = &orangepipc2Setup;

	orangepipc2->isr = &orangepipc2ISR;
	orangepipc2->waitForInterrupt = orangepipc2->soc->waitForInterrupt;

	orangepipc2->selectableFd = orangepipc2->soc->selectableFd;
	orangepipc2->gc = orangepipc2->soc->gc;

	orangepipc2->validGPIO = &orangepipc2ValidGPIO;
}
