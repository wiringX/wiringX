/*
  Copyright (c) 2023 Radxa Ltd.
  Author: Nascs <nascs@radxa.com>
          ZHANG Yuntian <yt@radxa.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "rock4.h"

struct platform_t *rock4 = NULL;

static int map[] = {
	/*	GPIO4_C2	GPIO4_A3	GPIO4_C6	GPIO4_C5	*/
			146,			131,			150,			149,
	/*	GPIO4_D2	GPIO4_D4	GPIO4_D5	GPIO2_B3	*/
			154,			156,			157,			75,
	/*	GPIO2_A7	GPIO2_B0	GPIO1_B2						*/
			71,				72,				42,				-1,
	/*	PIO1_B0		GPIO1_A7	GPIO1_B1	GPIO4_C4	*/
			40,				39,				41,				148,
	/*	PIO4_C3																	*/
			147,			-1,				-1,				-1,
	/*						GPIO2_B2	GPIO2_B1	GPIO2_B4	*/
			-1,				74,				73,				76,
	/*	GPIO4_A5	GPIO4_D6	GPIO3_C0	GPIO4_A4	*/
			133,			158,			112,			132,
	/*	GPIO4_A6	GPIO4_A7	GPIO2_A0	GPIO2_A1	*/
			134,			135,			64,				65
};

static int rock4ValidGPIO(int pin) {
	return radxaValidGPIO(pin, map, _sizeof(map));
}

static int rock4Setup(void) {
	return radxaSetup(rock4, map, _sizeof(map));
}

void rock4Init(void) {
	platform_register(&rock4, "rock4");

	rock4->soc = soc_get("Rockchip", "RK3399");
	rock4->soc->setMap(map, _sizeof(map));

	radxaInit(rock4);
	rock4->setup = &rock4Setup;

	rock4->validGPIO = &rock4ValidGPIO;
}
