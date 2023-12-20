/*
  Copyright (c) 2023 Radxa Ltd.
  Author: Nascs <nascs@radxa.com>
          ZHANG Yuntian <yt@radxa.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "rock5b.h"

struct platform_t *rock5b = NULL;

static int map[] = {
	/*	GPIO3_C1	GPIO3_B5	GPIO3_B7	GPIO3_C0	*/
			113,			109,			111,			112,
	/*	GPIO3_A4	GPIO4_C4						GPIO3_C3	*/
			100,			148,			-1,				115,
	/*	GPIO4_B3	GPIO4_B2	GPIO1_B4	GPIO1_B5	*/
			139,			138,			44,				45,
	/*	GPIO1_B2	GPIO1_B1	GPIO1_B3	GPIO0_B5	*/
			42,				41,				43,				13,
	/*	GPIO0_B6																*/
			14,				-1,				-1,				-1,
	/*						GPIO1_D7	GPIO1_B7	GPIO3_A7	*/
			-1,				63,				47,				103,
	/*	GPIO3_B6						GPIO3_C2	GPIO3_B1	*/
			110,			-1,				114,			105,
	/*	GPIO3_B2	GPIO3_B3	GPIO4_C6	GPIO4_C5	*/
			106,			107,			150,			149,
};

static int rock5bValidGPIO(int pin) {
	return radxaValidGPIO(pin, map, _sizeof(map));
}

static int rock5bSetup(void) {
	return radxaSetup(rock5b, map, _sizeof(map));
}

void rock5bInit(void) {
	platform_register(&rock5b,"rock5b");

	rock5b->soc = soc_get("Rockchip","RK3588");
	rock5b->soc->setMap(map,_sizeof(map));

	radxaInit(rock5b);
	rock5b->setup = &rock5bSetup;
	rock5b->validGPIO = &rock5bValidGPIO;
}
