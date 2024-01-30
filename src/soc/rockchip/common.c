/*
  Copyright (c) 2023 Radxa Ltd.
  Author: Nascs <nascs@radxa.com>
          ZHANG Yuntian <yt@radxa.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "common.h"

char *rockchipGetPinName(struct soc_t *soc, int pin) {
	return soc->layout[pin].name;
}

void rockchipSetMap(struct soc_t *soc, int *map, size_t size) {
	soc->map = map;
	soc->map_size = size;
}

void rockchipSetIRQ(struct soc_t *soc, int *irq, size_t size) {
	soc->irq = irq;
	soc->irq_size = size;
}

struct layout_t *rockchipGetLayout(struct soc_t *soc, int i, int *mapping) {
	struct layout_t *pin = NULL;
	unsigned int *grf_reg = NULL;
	unsigned int iomux_value = 0;

	if(mapping == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been mapped", soc->brand, soc->chip);
		return NULL;
	}
	if(wiringXValidGPIO(i) != 0) {
		wiringXLog(LOG_ERR, "The %i is not the right GPIO number");
		return NULL;
	}
	if(soc->fd <= 0 || soc->gpio == NULL) {
		wiringXLog(LOG_ERR, "The %s %s has not yet been setup by wiringX", soc->brand, soc->chip);
		return NULL;
	}

	pin = &soc->layout[mapping[i]];

	if(pin->grf.offset == GRF_UNDFEIND_IOMUX) {
		wiringXLog(LOG_ERR, "Pin %i is mapped to undefined pin on the hardware", i);
		return NULL;
	}

	return pin;
}

int rockchipDigitalRead(struct soc_t *soc, int i) {
	struct layout_t *pin = NULL;
	unsigned int *data_reg = NULL;
	uint32_t val = 0;

	if((pin = rockchipGetPinLayout(soc, i)) == NULL) {
		return -1;
	}

	if(pin->mode != PINMODE_INPUT) {
		wiringXLog(LOG_ERR, "The %s %s GPIO%d is not set to input mode", soc->brand, soc->chip, i);
		return -1;
	}

	data_reg = (volatile unsigned int *)(soc->gpio[pin->bank] + pin->in.offset);
	val = *data_reg;

	return (int)((val & (1 << pin->in.bit)) >> pin->in.bit);
}

int rockchipISR(struct soc_t *soc, int i, enum isr_mode_t mode) {
	struct layout_t *pin = NULL;
	char path[PATH_MAX];
	memset(path, 0, sizeof(path));

	if((pin = rockchipGetIrqLayout(soc, i)) == NULL) {
		return -1;
	}

	sprintf(path, "/sys/class/gpio/gpio%d", soc->irq[i]);
	if((soc_sysfs_check_gpio(soc, path)) == -1) {
		sprintf(path, "/sys/class/gpio/export");
		if(soc_sysfs_gpio_export(soc, path, soc->irq[i]) == -1) {
			return -1;
		}
	}

	sprintf(path, "/sys/class/gpio/gpio%d/direction", soc->irq[i]);
	if(soc_sysfs_set_gpio_direction(soc, path, "in") == -1) {
		return -1;
	}

	sprintf(path, "/sys/class/gpio/gpio%d/edge", soc->irq[i]);
	if(soc_sysfs_set_gpio_interrupt_mode(soc, path, mode) == -1) {
		return -1;
	}

	sprintf(path, "/sys/class/gpio/gpio%d/value", soc->irq[i]);
	if((pin->fd = soc_sysfs_gpio_reset_value(soc, path)) == -1) {
		return -1;
	}

	pin->mode = PINMODE_INTERRUPT;

	return 0;
}

int rockchipWaitForInterrupt(struct soc_t *soc, int i, int ms) {
	struct layout_t *pin = NULL;

	if((pin = rockchipGetIrqLayout(soc, i)) == NULL) {
		return -1;
	}

	if(pin->mode != PINMODE_INTERRUPT) {
		wiringXLog(LOG_ERR, "The %s %s GPIO %d is not set to interrupt mode", soc->brand, soc->chip, i);
		return -1;
	}

	return soc_wait_for_interrupt(soc, pin->fd, ms);
}

int rockchipGC(struct soc_t *soc) {
	struct layout_t *pin = NULL;
	char path[PATH_MAX] = { 0 };
	int i = 0;

	if(soc->map != NULL) {
		for(i = 0; i < soc->map_size; i++) {
			pin = &soc->layout[soc->map[i]];
			if(pin->mode == PINMODE_OUTPUT) {
				pinMode(i, PINMODE_INPUT);
			} else if(pin->mode == PINMODE_INTERRUPT) {
				sprintf(path, "/sys/class/gpio/gpio%d", soc->irq[i]);
				if((soc_sysfs_check_gpio(soc, path)) == 0) {
					sprintf(path, "/sys/class/gpio/unexport");
					soc_sysfs_gpio_unexport(soc, path, soc->irq[i]);
				}
			}

			if(pin->fd > 0) {
				close(pin->fd);
				pin->fd = 0;
			}
		}
	}
}

int rockchipSelectableFd(struct soc_t *soc, int i) {
	struct layout_t *pin = NULL;

	if((pin = rockchipGetIrqLayout(soc, (i)) == NULL)) {
		return -1;
	}

	return pin->fd;
}
