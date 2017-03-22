/*
    Copyright (c) 2017 Oleksii Serdiuk <contacts@oleksii.name>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <unistd.h>

#include "../../soc/soc.h"

#include "pinea64.h"

struct platform_t *pineA64 = NULL;

/*
Pi-2 Bus

   3.3V||5V
PH3 | 8||5V
PH2 | 9||GND
PL10| 7||15|PB0
    GND||16|PB1
PC7 | 0||1 |PC8
PH9 | 2||GND
PC12| 3||4 |PC13
   3.3V||5 |PC14
PC0 |12||GND
PC1 |13||6 |PC15
PC2 |14||10|PC3
    GND||11|PH7
PL9 |30||31|PL8
PH5 |21||GND
PH6 |22||26|PC4
PC5 |23||GND
PC9 |24||27|PC6
PC16|25||28|PC10
    GND||29|PC11

Euler Bus

   3.3V||DC IN
  Batt+||DC IN
   Temp||GND
PL11|48||5V
    GND||49|PH8
PB3 |43||44|PB4
PB5 |45||GND
PB6 |46||47|PB7
   3.3V||37|PD4
PD2 |39||GND
PD3 |40||38|PD5
PD1 |41||42|PD0
    GND||33|PD6
PB2 |32||34|PD7
PB8 |35||36|PB9
EAROUTP||EAROUT_N
   N.C.||GND

Exp bus

   3.3V||52|PL8
Chg.LED||Rst.Sw
 Pwr.Sw||GND
PB8 |50||51|PB9
    GND||KeyADC
*/

static int map[] = {
// Pi Bus: 0..31
/*	PC7  PC8  PH9  PC12 PC13 PC14 PC15 PL10 PH3  PH2	*/
	71,  72,  233, 76,  77,  78,  79,  362, 227, 226,
/*	PC3  PH7  PC0  PC1  PC2  PB0  PB1, --   --   --	*/
	67,  231, 64,  65,  66,  32,  33,  -1,  -1,  -1,
/*	--   PH5  PH6  PC5  PC9  PC16 PC4  PC6  PC10 PC11	*/
	-1,  229, 230, 69,  73,  80,  68,  70,  74,  75,
/*	PL9  PL8	*/
	361, 360,

// Euler bus: 32..49
/*	PB2  PD6  PD7  PB8  PB9  PD4  PD5  PD2  PD3  PD1	*/
	34,  102, 103, 40,  41,  100, 101,  98,  99,  97,
/*	PD0  PB3  PB4  PB5  PB6  PB7  PL11 PH8	*/
	96,  35,  36,  37,  38,  39,  363, 232,

// Exp bus: 50..52
/*	PB8  PB9  PL7	*/
	40,  41,  359
};

static int pineA64Setup(void) {
	const size_t size = sizeof(map) / sizeof(map[0]);
	if(pineA64->soc->setup() < 0)
		return -1;
	pineA64->soc->setMap(map, size);
	pineA64->soc->setIRQ(map, size);
	return 0;
}

static int pineA64ValidGPIO(int pin) {
	if(pin < 0 || pin >= (sizeof(map)/sizeof(map[0])) || map[pin] == -1) {
		return -1;
	} else {
		return 0;
	}
}

static int pineA64PinMode(int pin, enum pinmode_t mode) {
	if(pineA64ValidGPIO(pin) != 0) {
		return -1;
	}
	return pineA64->soc->pinMode(pin, mode);
}

static int pineA64DigitalWrite(int pin, enum digital_value_t value) {
	if(pineA64ValidGPIO(pin) != 0) {
		return -1;
	}
	return pineA64->soc->digitalWrite(pin, value);
}

static int pineA64DigitalRead(int pin) {
	if(pineA64ValidGPIO(pin) != 0) {
		return -1;
	}
	return pineA64->soc->digitalRead(pin);
}

static int pineA64ISR(int pin, enum isr_mode_t mode) {
	if(pineA64ValidGPIO(pin) != 0) {
		return -1;
	}
	return pineA64->soc->isr(pin, mode);
}

static int pineA64WaitForInterrupt(int pin, int ms) {
	if(pineA64ValidGPIO(pin) != 0) {
		return -1;
	}
	return pineA64->soc->waitForInterrupt(pin, ms);
}

void pineA64Init(void) {
	platform_register(&pineA64, "pinea64");

	pineA64->soc = soc_get("Allwinner", "A64");

	pineA64->setup = &pineA64Setup;
	pineA64->pinMode = &pineA64PinMode;
	pineA64->digitalWrite = &pineA64DigitalWrite;
	pineA64->digitalRead = &pineA64DigitalRead;
	pineA64->waitForInterrupt = &pineA64WaitForInterrupt;
	pineA64->isr = &pineA64ISR;

	pineA64->validGPIO = &pineA64ValidGPIO;
	pineA64->gc = pineA64->soc->gc;
}
