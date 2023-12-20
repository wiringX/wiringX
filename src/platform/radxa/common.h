/*
  Copyright (c) 2023 Radxa Ltd.
  Author: Nascs <nascs@radxa.com>
          ZHANG Yuntian <yt@radxa.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __WIRINGX_RADXA_COMMON_H_
#define __WIRINGX_RADXA_COMMON_H_

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

#define _sizeof(arr) (sizeof(arr) / sizeof(arr[0]))

int radxaValidGPIO(int pin, int *map, int map_count);
int radxaSetup(struct platform_t *platform, int *map, int map_count);
void radxaInit(struct platform_t *platform);

#endif
