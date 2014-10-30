/*
	Copyright (c) 2014 CurlyMo <curlymoo1@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "wiringX.h"
#include "hummingboard.h"
#include "raspberrypi.h"
#include "bananapi.h"
#include "radxa.h"

struct devices_t *device = NULL;
static int setup = -2;

void device_register(struct devices_t **dev, const char *name) {
	*dev = malloc(sizeof(struct devices_t));
	(*dev)->pinMode = NULL;
	(*dev)->digitalWrite = NULL;
	(*dev)->digitalRead = NULL;
	(*dev)->identify = NULL;
	(*dev)->waitForInterrupt = NULL;
	(*dev)->isr = NULL;
	(*dev)->I2CRead = NULL;
	(*dev)->I2CReadReg8 = NULL;
	(*dev)->I2CReadReg16 = NULL;
	(*dev)->I2CWrite = NULL;
	(*dev)->I2CWriteReg8 = NULL;
	(*dev)->I2CWriteReg16 = NULL;

	if(!((*dev)->name = malloc(strlen(name)+1))) {
		fprintf(stderr, "wiringX: out of memory\n");
		exit(0);
	}
	strcpy((*dev)->name, name);
	(*dev)->next = devices;
	devices = (*dev);
}

int wiringXGC(void) {
	int i = 0;
	if(device) {
		i = device->gc();
	}
	device = NULL;
	struct devices_t *tmp = devices;
	while(devices) {
		tmp = devices;
		free(devices->name);
		devices = devices->next;
		free(tmp);
	}
	free(devices);
	return i;
}

void pinMode(int pin, int mode) {
	if(device) {
		if(device->pinMode) {
			if(device->pinMode(pin, mode) == -1) {
				fprintf(stderr, "%s: error while calling pinMode\n", device->name);
				wiringXGC();
			}
		} else {
			fprintf(stderr, "%s: device doesn't support pinMode\n", device->name);
			wiringXGC();
		}
	}
}

void digitalWrite(int pin, int value) {
	if(device) {
		if(device->digitalWrite) {
			if(device->digitalWrite(pin, value) == -1) {
				fprintf(stderr, "%s: error while calling digitalWrite\n", device->name);
				wiringXGC();
			}
		} else {
			fprintf(stderr, "%s: device doesn't support digitalWrite\n", device->name);
			wiringXGC();
		}
	}
}

int digitalRead(int pin) {
	if(device) {
		if(device->digitalRead) {
			int x = device->digitalRead(pin);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling digitalRead\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support digitalRead\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int waitForInterrupt(int pin, int ms) {
	if(device) {
		if(device->waitForInterrupt) {
			int x = device->waitForInterrupt(pin, ms);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling waitForInterrupt\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support waitForInterrupt\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXISR(int pin, int mode) {
	if(device) {
		if(device->isr) {
			int x = device->isr(pin, mode);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling isr\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support isr\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CRead(int fd) {
	if(device) {
		if(device->I2CRead) {
			int x = device->I2CRead(fd);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CRead\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CRead\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CReadReg8(int fd, int reg) {
	if(device) {
		if(device->I2CReadReg8) {
			int x = device->I2CReadReg8(fd, reg);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CReadReg8\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CReadReg8\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CReadReg16(int fd, int reg) {
	if(device) {
		if(device->I2CReadReg16) {
			int x = device->I2CReadReg16(fd, reg);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CReadReg16\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CReadReg16\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CWrite(int fd, int data) {
	if(device) {
		if(device->I2CWrite) {
			int x = device->I2CWrite(fd, data);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CWrite\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CWrite\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CWriteReg8(int fd, int reg, int data) {
	if(device) {
		if(device->I2CWriteReg8) {
			int x = device->I2CWriteReg8(fd, reg, data);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CWriteReg8\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CWriteReg8\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CWriteReg16(int fd, int reg, int data) {
	if(device) {
		if(device->I2CWriteReg16) {
			int x = device->I2CWriteReg16(fd, reg, data);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CWriteReg16\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CWriteReg16\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CSetup(int devId) {
	if(device) {
		if(device->I2CSetup) {
			int x = device->I2CSetup(devId);
			if(x == -1) {
				fprintf(stderr, "%s: error while calling I2CSetup\n", device->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			fprintf(stderr, "%s: device doesn't support I2CSetup\n", device->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXSetup(void) {
	if(setup == -2) {
		hummingboardInit();
		raspberrypiInit();
		bananapiInit();
		radxaInit();

		int match = 0;
		struct devices_t *tmp = devices;
		while(tmp) {
			if(tmp->identify() >= 0) {
				device = tmp;
				match = 1;
				break;
			}
			tmp = tmp->next;
		}

		if(match == 0) {
			fprintf(stderr, "wiringX: hardware not supported\n");
			wiringXGC();
			return -1;
		}
		printf("running on a %s\n", device->name);
		setup = device->setup();
		return setup;
	} else {
		return setup;
	}
}
