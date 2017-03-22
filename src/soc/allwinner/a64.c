/*
	Copyright (c) 2017 Oleksii Serdiuk <contacts@oleksii.name>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "../../wiringX.h"
#include "../soc.h"

#include "a64.h"

#define PAGE_SIZE	4096

// From Allwinner_A64_User_Manual_V1.0.pdf
// Page 376, ports PB-PH
#define CPUx_PORT_BASE_ADDRESS	0x01C20000
#define CPUx_PORT_GPIO_OFFSET	0x00000800
// Page 410, port PL
#define CPUs_PORT_BASE_ADDRESS	0x01F02000
#define CPUs_PORT_GPIO_OFFSET	0x00000C00

#define PIN_MODE_INPUT	0x0
#define PIN_MODE_OUTPUT	0x1
#define PIN_MODE_INTERRUPT	0x6
#define PIN_MODE_DISABLE	0x7

#define LAYOUT_OFFSET	32
#define NULLREC	{ NULL, -1, {}, {}, FUNCTION_UNKNOWN, PINMODE_NOT_SET, -1 }

struct soc_t *allwinnerA64 = NULL;

static struct layout_t {
	char *name;

	int addr;

	struct {
		uintptr_t offset;
		unsigned short bit;
	} select;

	struct {
		uintptr_t offset;
		unsigned short bit;
	} data;

	int support;

	enum pinmode_t mode;

	int fd;
} layout[] = {
// 32
 { "PB0", 0, { 0x24, 0 }, { 0x34, 0 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB1", 0, { 0x24, 4 }, { 0x34, 1 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB2", 0, { 0x24, 8 }, { 0x34, 2 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB3", 0, { 0x24, 12 }, { 0x34, 3 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB4", 0, { 0x24, 16 }, { 0x34, 4 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB5", 0, { 0x24, 20 }, { 0x34, 5 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB6", 0, { 0x24, 24 }, { 0x34, 6 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB7", 0, { 0x24, 28 }, { 0x34, 7 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB8", 0, { 0x28, 0 }, { 0x34, 8 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PB9", 0, { 0x28, 4 }, { 0x34, 9 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 64
 { "PC0", 0, { 0x48, 0 }, { 0x58, 0 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC1", 0, { 0x48, 4 }, { 0x58, 1 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC2", 0, { 0x48, 8 }, { 0x58, 2 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC3", 0, { 0x48, 12 }, { 0x58, 3 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC4", 0, { 0x48, 16 }, { 0x58, 4 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC5", 0, { 0x48, 20 }, { 0x58, 5 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC6", 0, { 0x48, 24 }, { 0x58, 6 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC7", 0, { 0x48, 28 }, { 0x58, 7 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC8", 0, { 0x4C, 0 }, { 0x58, 8 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC9", 0, { 0x4C, 4 }, { 0x58, 9 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC10", 0, { 0x4C, 8 }, { 0x58, 10 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC11", 0, { 0x4C, 12 }, { 0x58, 11 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC12", 0, { 0x4C, 16 }, { 0x58, 12 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC13", 0, { 0x4C, 20 }, { 0x58, 13 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC14", 0, { 0x4C, 24 }, { 0x58, 14 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC15", 0, { 0x4C, 28 }, { 0x58, 15 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PC16", 0, { 0x50, 0 }, { 0x58, 16 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 96
 { "PD0", 0, { 0x6C, 0 }, { 0x7C, 0 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD1", 0, { 0x6C, 4 }, { 0x7C, 1 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD2", 0, { 0x6C, 8 }, { 0x7C, 2 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD3", 0, { 0x6C, 12 }, { 0x7C, 3 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD4", 0, { 0x6C, 16 }, { 0x7C, 4 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD5", 0, { 0x6C, 20 }, { 0x7C, 5 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD6", 0, { 0x6C, 24 }, { 0x7C, 6 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD7", 0, { 0x6C, 28 }, { 0x7C, 7 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD8", 0, { 0x70, 0 }, { 0x7C, 8 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD9", 0, { 0x70, 4 }, { 0x7C, 9 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD10", 0, { 0x70, 8 }, { 0x7C, 10 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD11", 0, { 0x70, 12 }, { 0x7C, 11 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD12", 0, { 0x70, 16 }, { 0x7C, 12 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD13", 0, { 0x70, 20 }, { 0x7C, 13 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD14", 0, { 0x70, 24 }, { 0x7C, 14 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD15", 0, { 0x70, 28 }, { 0x7C, 15 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD16", 0, { 0x74, 0 }, { 0x7C, 16 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD17", 0, { 0x74, 4 }, { 0x7C, 17 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD18", 0, { 0x74, 8 }, { 0x7C, 18 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD19", 0, { 0x74, 12 }, { 0x7C, 19 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD20", 0, { 0x74, 16 }, { 0x7C, 20 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD21", 0, { 0x74, 20 }, { 0x7C, 21 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD22", 0, { 0x74, 24 }, { 0x7C, 22 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD23", 0, { 0x74, 28 }, { 0x7C, 23 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PD24", 0, { 0x78, 0 }, { 0x7C, 24 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 128
 { "PE0", 0, { 0x90, 0 }, { 0xA0, 0 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE1", 0, { 0x90, 4 }, { 0xA0, 1 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE2", 0, { 0x90, 8 }, { 0xA0, 2 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE3", 0, { 0x90, 12 }, { 0xA0, 3 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE4", 0, { 0x90, 16 }, { 0xA0, 4 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE5", 0, { 0x90, 20 }, { 0xA0, 5 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE6", 0, { 0x90, 24 }, { 0xA0, 6 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE7", 0, { 0x90, 28 }, { 0xA0, 7 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE8", 0, { 0x94, 0 }, { 0xA0, 8 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE9", 0, { 0x94, 4 }, { 0xA0, 9 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE10", 0, { 0x94, 8 }, { 0xA0, 10 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE11", 0, { 0x94, 12 }, { 0xA0, 11 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE12", 0, { 0x94, 16 }, { 0xA0, 12 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE13", 0, { 0x94, 20 }, { 0xA0, 13 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE14", 0, { 0x94, 24 }, { 0xA0, 14 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE15", 0, { 0x94, 28 }, { 0xA0, 15 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE16", 0, { 0x98, 28 }, { 0xA0, 16 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PE17", 0, { 0x98, 28 }, { 0xA0, 17 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 160
 { "PF0", 0, { 0xB4, 0 }, { 0xC4, 0 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PF1", 0, { 0xB4, 4 }, { 0xC4, 1 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PF2", 0, { 0xB4, 8 }, { 0xC4, 2 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PF3", 0, { 0xB4, 12 }, { 0xC4, 3 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PF4", 0, { 0xB4, 16 }, { 0xC4, 4 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PF5", 0, { 0xB4, 20 }, { 0xC4, 5 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 { "PF6", 0, { 0xB4, 24 }, { 0xC4, 6 }, FUNCTION_DIGITAL, PINMODE_NOT_SET, 0 },
 NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 192
 { "PG0", 0, { 0xD8, 0 }, { 0xE8, 0 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG1", 0, { 0xD8, 4 }, { 0xE8, 1 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG2", 0, { 0xD8, 8 }, { 0xE8, 2 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG3", 0, { 0xD8, 12 }, { 0xE8, 3 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG4", 0, { 0xD8, 16 }, { 0xE8, 4 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG5", 0, { 0xD8, 20 }, { 0xE8, 5 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG6", 0, { 0xD8, 24 }, { 0xE8, 6 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG7", 0, { 0xD8, 28 }, { 0xE8, 7 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG8", 0, { 0xDC, 0 }, { 0xE8, 8 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG9", 0, { 0xDC, 4 }, { 0xE8, 9 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG10", 0, { 0xDC, 8 }, { 0xE8, 10 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG11", 0, { 0xDC, 12 }, { 0xE8, 11 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG12", 0, { 0xDC, 16 }, { 0xE8, 12 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PG13", 0, { 0xDC, 20 }, { 0xE8, 13 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 224
 { "PH0", 0, { 0xFC, 0 }, { 0x10C, 0 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH1", 0, { 0xFC, 4 }, { 0x10C, 1 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH2", 0, { 0xFC, 8 }, { 0x10C, 2 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH3", 0, { 0xFC, 12 }, { 0x10C, 3 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH4", 0, { 0xFC, 16 }, { 0x10C, 4 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH5", 0, { 0xFC, 20 }, { 0x10C, 5 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH6", 0, { 0xFC, 24 }, { 0x10C, 6 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH7", 0, { 0xFC, 28 }, { 0x10C, 7 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH8", 0, { 0x100, 0 }, { 0x10C, 8 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH9", 0, { 0x100, 4 }, { 0x10C, 9 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH10", 0, { 0x100, 8 }, { 0x10C, 10 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PH11", 0, { 0x100, 12 }, { 0x10C, 11 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 256
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 288
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 320
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
 NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC, NULLREC,
// 352
 { "PL0", 1, { 0x00, 0 }, { 0x10, 0 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL1", 1, { 0x00, 4 }, { 0x10, 1 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL2", 1, { 0x00, 8 }, { 0x10, 2 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL3", 1, { 0x00, 12 }, { 0x10, 3 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL4", 1, { 0x00, 16 }, { 0x10, 4 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL5", 1, { 0x00, 20 }, { 0x10, 5 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL6", 1, { 0x00, 24 }, { 0x10, 6 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL7", 1, { 0x00, 28 }, { 0x10, 7 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL8", 1, { 0x04, 0 }, { 0x10, 8 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL9", 1, { 0x04, 4 }, { 0x10, 9 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL10", 1, { 0x04, 8 }, { 0x10, 10 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL11", 1, { 0x04, 12 }, { 0x10, 11 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
 { "PL12", 1, { 0x04, 16 }, { 0x10, 12 }, FUNCTION_DIGITAL | FUNCTION_INTERRUPT, PINMODE_NOT_SET, 0 },
};

static struct layout_t *allwinnerA64GetPinStruct(int pin) {
	if(allwinnerA64->map == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been mapped",
			allwinnerA64->brand, allwinnerA64->chip);
		return NULL;
	}

	return &allwinnerA64->layout[allwinnerA64->map[pin] - LAYOUT_OFFSET];
}

static uintptr_t allwinnerA64GetAddressForPin(struct layout_t *pin, uintptr_t offset) {
	void *gpio = NULL;

	gpio = allwinnerA64->gpio[pin->addr];
	if(allwinnerA64->fd <= 0 || gpio == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been setup by wiringX",
			allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}

	return (uintptr_t)(gpio + allwinnerA64->base_offs[pin->addr] + offset);
}

static char *allwinnerA64GetPinName(int pin) {
	return allwinnerA64->layout[pin].name;
}

static int allwinnerA64DigitalWrite(int i, enum digital_value_t value) {
	struct layout_t *pin = NULL;
	uintptr_t addr = 0;
	uint32_t val = 0;

	pin = allwinnerA64GetPinStruct(i);
	if(pin == NULL) {
		return -1;
	}

	addr = allwinnerA64GetAddressForPin(pin, pin->data.offset);
	if(addr < 0) {
		return -1;
	}

	if(pin->mode != PINMODE_OUTPUT) {
		wiringXLog(LOG_ERR, "The %s %s GPIO %d is not set to output mode",
			allwinnerA64->brand, allwinnerA64->chip, i);
		return -1;
	}

	val = soc_readl(addr);
	if(value == HIGH) {
		val |= (1 << pin->data.bit);
	} else {
		val &= ~(1 << pin->data.bit);
	}
	soc_writel(addr, val);
	return 0;
}

static int allwinnerA64DigitalRead(int i) {
	struct layout_t *pin = NULL;
	uintptr_t addr = 0;
	uint32_t val = 0;

	pin = allwinnerA64GetPinStruct(i);
	if(pin == NULL) {
		return -1;
	}

	addr = allwinnerA64GetAddressForPin(pin, pin->data.offset);
	if(addr < 0) {
		return -1;
	}

	if(pin->mode != PINMODE_INPUT) {
		wiringXLog(LOG_ERR, "The %s %s GPIO %d is not set to input mode",
			allwinnerA64->brand, allwinnerA64->chip, i);
		return -1;
	}

	val = soc_readl(addr);

	return (int)((val & (1 << pin->data.bit)) >> pin->data.bit);
}

static int allwinnerA64PinMode(int i, enum pinmode_t mode) {
	struct layout_t *pin = NULL;
	void *gpio = NULL;
	uintptr_t addr = 0;
	uint32_t val = 0;

	pin = allwinnerA64GetPinStruct(i);
	if(pin == NULL) {
		return -1;
	}

	gpio = allwinnerA64->gpio[pin->addr];

	if(allwinnerA64->fd <= 0 || gpio == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been setup by wiringX",
			allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}

	addr = (uintptr_t)gpio + allwinnerA64->base_offs[pin->addr] + pin->select.offset;

	val = soc_readl(addr);
	// Reset pin bits to 0
	val &= ~(0xF << pin->select.bit);
	switch(mode){
	case PINMODE_NOT_SET:
		val |= (PIN_MODE_DISABLE << pin->select.bit);
		break;
	case PINMODE_INPUT:
		val |= (PIN_MODE_INPUT << pin->select.bit);
		break;
	case PINMODE_OUTPUT:
		val |= (PIN_MODE_OUTPUT << pin->select.bit);
		break;
	default:
		wiringXLog(LOG_ERR, "The %s %s GPIO %d doesn't support mode %d",
			allwinnerA64->brand, allwinnerA64->chip, i, mode);
		return -1;
	}
	soc_writel(addr, val);
	pin->mode = mode;

	return 0;
}

static int allwinnerA64ISR(int i, enum isr_mode_t mode) {
	struct layout_t *pin = NULL;
	char path[PATH_MAX];

	if(allwinnerA64->irq == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been mapped",
			allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}
	if(allwinnerA64->fd <= 0 || allwinnerA64->gpio == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been setup by wiringX",
			allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}

	pin = &allwinnerA64->layout[allwinnerA64->irq[i] - LAYOUT_OFFSET];
	if(pin->support & FUNCTION_INTERRUPT == 0) {
		wiringXLog(LOG_ERR, "The %s %s GPIO %d doesn't support interrupts, try another pin",
			allwinnerA64->brand, allwinnerA64->chip, i);
		return -1;
	}

	sprintf(path, "/sys/class/gpio/gpio%d", allwinnerA64->irq[i]);
	if((soc_sysfs_check_gpio(allwinnerA64, path)) == -1) {
		sprintf(path, "/sys/class/gpio/export");
		if(soc_sysfs_gpio_export(allwinnerA64, path, allwinnerA64->irq[i]) == -1) {
			return -1;
		}
	}

	sprintf(path, "/sys/class/gpio/gpio%d/direction", allwinnerA64->irq[i]);
	if(soc_sysfs_set_gpio_direction(allwinnerA64, path, "in") == -1) {
		return -1;
	}

	sprintf(path, "/sys/class/gpio/gpio%d/edge", allwinnerA64->irq[i]);
	if(soc_sysfs_set_gpio_interrupt_mode(allwinnerA64, path, mode) == -1) {
		return -1;
	}

	sprintf(path, "/sys/class/gpio/gpio%d/value", allwinnerA64->irq[i]);
	if((pin->fd = soc_sysfs_gpio_reset_value(allwinnerA64, path)) == -1) {
		return -1;
	}
	pin->mode = PINMODE_INTERRUPT;

	return 0;
}


static int allwinnerA64WaitForInterrupt(int i, int ms) {
	struct layout_t *pin = NULL;

	if(allwinnerA64->irq == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been mapped",
				   allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}
	if(allwinnerA64->fd <= 0 || allwinnerA64->gpio == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been setup by wiringX",
			allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}

	pin = &allwinnerA64->layout[allwinnerA64->irq[i] - LAYOUT_OFFSET];
	if(pin->mode != PINMODE_INTERRUPT) {
		wiringXLog(LOG_ERR, "The %s %s GPIO %d is not set to interrupt mode",
			allwinnerA64->brand, allwinnerA64->chip, i);
		return -1;
	}
	if(pin->fd <= 0) {
		wiringXLog(LOG_ERR, "The %s %s GPIO %d has not been opened for reading",
			allwinnerA64->brand, allwinnerA64->chip, i);
		return -1;
	}

	return soc_wait_for_interrupt(allwinnerA64, pin->fd, ms);
}

static int allwinnerA64Setup(void) {
	if((allwinnerA64->fd = open("/dev/mem", O_RDWR | O_SYNC )) < 0) {
		wiringXLog(LOG_ERR, "wiringX failed to open /dev/mem for raw memory access");
		return -1;
	}

	if((allwinnerA64->gpio[0] = (unsigned char *)mmap(0,
			allwinnerA64->page_size,
			PROT_READ|PROT_WRITE, MAP_SHARED,
			allwinnerA64->fd,
			allwinnerA64->base_addr[0])) == NULL) {
		wiringXLog(LOG_ERR, "wiringX failed to map the %s %s GPIO memory address",
					allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}

	if((allwinnerA64->gpio[1] = (unsigned char *)mmap(0, allwinnerA64->page_size,
			PROT_READ|PROT_WRITE, MAP_SHARED,
			allwinnerA64->fd,
			allwinnerA64->base_addr[1])) == NULL) {
		wiringXLog(LOG_ERR, "wiringX failed to map the %s %s GPIO memory address",
					allwinnerA64->brand, allwinnerA64->chip);
		return -1;
	}

	return 0;
}

static void allwinnerA64SetMap(int *map, size_t size) {
	allwinnerA64->map = map;
	allwinnerA64->map_size = size;
}

static void allwinnerA64SetIRQ(int *irq, size_t size) {
	allwinnerA64->irq = irq;
	allwinnerA64->irq_size = size;
}

static int allwinnerA64GC(void) {
	struct layout_t *pin = NULL;
	char path[PATH_MAX];
	int i = 0;

	if(allwinnerA64->map != NULL) {
		for(i=0;i<allwinnerA64->map_size;i++) {
			pin = allwinnerA64GetPinStruct(i);
			if(pin->mode == PINMODE_INPUT || pin->mode == PINMODE_OUTPUT) {
				pinMode(i, PINMODE_NOT_SET);
			} else if(pin->mode == PINMODE_INTERRUPT) {
				sprintf(path, "/sys/class/gpio/gpio%d", allwinnerA64->irq[i]);
				if((soc_sysfs_check_gpio(allwinnerA64, path)) == 0) {
					sprintf(path, "/sys/class/gpio/unexport");
					soc_sysfs_gpio_unexport(allwinnerA64, path, allwinnerA64->irq[i]);
				}
			}
			if(pin->fd > 0) {
				close(pin->fd);
				pin->fd = 0;
			}
		}
	}
	if(allwinnerA64->gpio[0] != NULL) {
		munmap(allwinnerA64->gpio[0], allwinnerA64->page_size);
	}
	if(allwinnerA64->gpio[1] != NULL) {
		munmap(allwinnerA64->gpio[1], allwinnerA64->page_size);
	}
	return 0;
}

void allwinnerA64Init(void) {
	soc_register(&allwinnerA64, "Allwinner", "A64");

	allwinnerA64->layout = layout;

	allwinnerA64->support.isr_modes
		= ISR_MODE_RISING | ISR_MODE_FALLING | ISR_MODE_BOTH | ISR_MODE_NONE;

	allwinnerA64->page_size = PAGE_SIZE;
	allwinnerA64->base_addr[0] = CPUx_PORT_BASE_ADDRESS;
	allwinnerA64->base_offs[0] = CPUx_PORT_GPIO_OFFSET;

	allwinnerA64->base_addr[1] = CPUs_PORT_BASE_ADDRESS;
	allwinnerA64->base_offs[1] = CPUs_PORT_GPIO_OFFSET;

	allwinnerA64->digitalWrite = &allwinnerA64DigitalWrite;
	allwinnerA64->digitalRead = &allwinnerA64DigitalRead;
	allwinnerA64->pinMode = &allwinnerA64PinMode;
	allwinnerA64->isr = &allwinnerA64ISR;
	allwinnerA64->waitForInterrupt = &allwinnerA64WaitForInterrupt;

	allwinnerA64->setup = &allwinnerA64Setup;
	allwinnerA64->setMap = &allwinnerA64SetMap;
	allwinnerA64->setIRQ = &allwinnerA64SetIRQ;
	allwinnerA64->getPinName = &allwinnerA64GetPinName;

	allwinnerA64->gc = &allwinnerA64GC;
}
