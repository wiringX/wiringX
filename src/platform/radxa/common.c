/*
  Copyright (c) 2023 Radxa Ltd.
  Author: Nascs <nascs@radxa.com>
          ZHANG Yuntian <yt@radxa.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "common.h"

int radxaValidGPIO(int pin, int *map, int map_count) {
	return (pin < 0 || pin >= map_count || map[pin] == -1) ? -1 : 0;
}

int radxaSetup(struct platform_t *platform, int *map, int map_count) {
	int ret = 0;

	ret = platform->soc->setup();
	if(ret == 0) {
		platform->soc->setMap(map, map_count);
		platform->soc->setIRQ(map, map_count);
	}

	return ret;
}

void radxaInit(struct platform_t *platform) {
	platform->digitalRead = platform->soc->digitalRead;
	platform->digitalWrite = platform->soc->digitalWrite;
	platform->pinMode = platform->soc->pinMode;

	platform->isr = platform->soc->isr;
	platform->waitForInterrupt = platform->soc->waitForInterrupt;

	platform->selectableFd = platform->soc->selectableFd;
	platform->gc = platform->soc->gc;
}
