/*
  Copyright (c) 2023 Radxa Ltd.
  Author: Nascs <nascs@radxa.com>
          ZHANG Yuntian <yt@radxa.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __WIRINGX_ROCKCHIP_COMMON_H
#define __WIRINGX_ROCKCHIP_COMMON_H

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../wiringx.h"
#include "../soc.h"
#include "../../i2c-dev.h"

struct layout_t {
	char *name;
	int bank;
	int iomux_num;

	struct {
		unsigned long offset;
		unsigned long bit;
	} cru;
	struct {
		unsigned long offset;
		unsigned long bit;
	} grf;
	struct {
		unsigned long offset;
		unsigned long bit;
	} direction;
	struct {
		unsigned long offset;
		unsigned long bit;
	} out;
	struct {
		unsigned long offset;
		unsigned long bit;
	} in;
	int support;
	enum pinmode_t mode;
	int fd;
};

#define GRF_UNDFEIND_IOMUX	0xffff

#define REGISTER_WRITE_MASK	16			// High 16 bits are write mask,
// with 1 enabling the coressponding lower bit
// Set lower bit to 0 and higher bit (write mask) to 1

#define REGISTER_CLEAR_BITS(addr, bit, size) \
	(*addr = *addr & ~(~(-1 << size) << bit) | (~(-1 << size) << bit << REGISTER_WRITE_MASK))
#define REGISTER_SET_HIGH(addr, bit, clear_bit_num) \
	(*addr = *addr | (clear_bit_num << bit) | (clear_bit_num << bit << REGISTER_WRITE_MASK))
#define REGISTER_GET_BITS(addr, bit, size) \
	((*addr & ~(-1 << size) << bit) >> (bit - size))

char *rockchipGetPinName(struct soc_t *soc, int pin);
void rockchipSetMap(struct soc_t *soc, int *map, size_t size);
void rockchipSetIRQ(struct soc_t *soc, int *irq, size_t size);
struct layout_t *rockchipGetLayout(struct soc_t *soc, int i, int *mapping);
int rockchipDigitalRead(struct soc_t *soc, int i);
int rockchipISR(struct soc_t *soc, int i, enum isr_mode_t mode);
int rockchipWaitForInterrupt(struct soc_t *soc, int i, int ms);
int rockchipGC(struct soc_t *soc);
int rockchipSelectableFd(struct soc_t *soc, int i);

#define rockchipGetPinLayout(soc, i) (rockchipGetLayout(soc, i, soc->map))
#define rockchipGetIrqLayout(soc, i) (rockchipGetLayout(soc, i, soc->irq))
#define rockchip_mmap(soc, offset) (mmap(0, soc->page_size, PROT_READ | PROT_WRITE, MAP_SHARED, soc->fd, offset))

#endif
