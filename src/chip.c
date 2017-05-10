/*
	Copyright (c) 	2015 Gary Sims
					2014 CurlyMo <curlymoo1@gmail.com>
					2012 Gordon Henderson

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#ifndef __FreeBSD__
	#include <linux/spi/spidev.h>
#endif

#include "wiringX.h"
#ifndef __FreeBSD__
	#include "i2c-dev.h"
#endif
#include "chip.h"

#define NUM_PINS	37

#define	GPIO_BASE	0x01C20800 // not at 4k border e.g. 0x01c20000
#define	GPIO_OFFSET	0x00000800 // add to 4k border
#define PAGE_SIZE	0x1000
#define PAGE_MASK 	(PAGE_SIZE - 1)

static char * channelToDevice[] = { "/dev/i2c-0", 
								    "/dev/i2c-1", 
									"/dev/i2c-2", 
									"/dev/null" };

static int pinModes[NUM_PINS];

static int pinToGpio[NUM_PINS] = {
		408, // wiringX #  0 - Physical U14-13 - XIO-P0  // access > 120µs (I2C proxy)
		409, // wiringX #  1 - Physical U14-14 - XIO-P1  // access > 120µs (I2C proxy)
		410, // wiringX #  2 - Physical U14-15 - XIO-P2  // access > 120µs (I2C proxy)
		411, // wiringX #  3 - Physical U14-16 - XIO-P3  // access > 120µs (I2C proxy)
		412, // wiringX #  4 - Physical U14-17 - XIO-P4  // access > 120µs (I2C proxy)
		413, // wiringX #  5 - Physical U14-18 - XIO-P5  // access > 120µs (I2C proxy)
		414, // wiringX #  6 - Physical U14-19 - XIO-P6  // access > 120µs (I2C proxy)
		415, // wiringX #  7 - Physical U14-20 - XIO-P7  // access > 120µs (I2C proxy)

		//128, // wiringX #  - Physical U14-27 - CSIPCK   (PE-0) input only
		//129, // wiringX #  - Physical U14-28 - CSICK    (PE-1) input only
		//130, // wiringX #  - Physical U14-29 - CSIHSYNC (PE-2) input only
		//131, // wiringX #  - Physical U14-30 - CSIVSYNC (PE-3) input only ??
		132, // wiringX #  8 - Physical U14-31 - CSID0 (PE-4)
		133, // wiringX #  9 - Physical U14-32 - CSID1 (PE-5)
		134, // wiringX # 10 - Physical U14-33 - CSID2 (PE-6) 
		135, // wiringX # 11 - Physical U14-34 - CSID3 (PE-7)
		136, // wiringX # 12 - Physical U14-35 - CSID4 (PE-8)
		137, // wiringX # 13 - Physical U14-36 - CSID5 (PE-9)
		138, // wiringX # 14 - Physical U14-37 - CSID6 (PE-10)
		139, // wiringX # 15 - Physical U14-38 - CSID7 (PE-11)

		 99, // wiringX # 16 - Physical U13-20 - LCD-D3 (PD-3)
		100, // wiringX # 17 - Physical U13-19 - LCD-D4 ..
		101, // wiringX # 18 - Physical U13-22 - LCD-D5
		102, // wiringX # 19 - Physical U13-21 - LCD-D6
		103, // wiringX # 20 - Physical U13-24 - LCD-D7
		106, // wiringX # 21 - Physical U13-23 - LCD-D10
		107, // wiringX # 22 - Physical U13-26 - LCD-D11
		108, // wiringX # 23 - Physical U13-25 - LCD-D12
		109, // wiringX # 24 - Physical U13-28 - LCD-D13
		110, // wiringX # 25 - Physical U13-27 - LCD-D14
		111, // wiringX # 26 - Physical U13-30 - LCD-D15
		114, // wiringX # 27 - Physical U13-29 - LCD-D18
		115, // wiringX # 28 - Physical U13-32 - LCD-D19
		116, // wiringX # 29 - Physical U13-31 - LCD-D20
		117, // wiringX # 30 - Physical U13-34 - LCD-D21
		118, // wiringX # 31 - Physical U13-33 - LCD-D22 ..
		119, // wiringX # 32 - Physical U13-36 - LCD-D23 (PD-23)

		120, // wiringX # 33 - Physical U13-35 - LCD-CLK   (PD-24)
		122, // wiringX # 34 - Physical U13-38 - LCD-HSYNC (PD-26)
		123, // wiringX # 35 - Physical U13-37 - LCD-VSYNC (PD-27)
		121, // wiringX # 36 - Physical U13-40 - LCD-DE    (PD-25)
};

//********************************************************************
// PinMode Registers: Offset and bit
static int pinToRegMode[] = { 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // no MMAP access for XIO 0-7 (I2C proxy)
	0x90, 0x90, 0x90, 0x90, 0x94, 0x94, 0x94, 0x94, // CSID0-7 (PE 4..11)
	0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x70, 0x70, 0x70, // 16..23 (PD-3..7,10..12)
	0x70, 0x70, 0x70, 0x74, 0x74, 0x74, 0x74, 0x74, // 24..31 (PD-13..15,18..22)
	0x74, 0x78, 0x78, 0x78, 0x78,   -1,   -1,   -1, // 32..36 (PD-23,24,26,27,25)
};
static int pinToRegMBit[] = { 
     0,  0,  0,  0,  0,  0,  0,  0, // no mmap access for XIO 0-7
    16, 20, 24, 28,  0,  4,  8, 12, // 8..15   (PE-4..11)
    12, 16, 20, 24, 28,  8, 12, 16, // 16..23  (PD-3..7,10..12)
    20, 24, 28,  8, 12, 16, 20, 24, // 24..31  (PD-13..15,18..22)
    28,  0,  8, 12,  4,  0,  0,  0, // 32..36  (PD-23,24,26,27,25)
};

//********************************************************************
// Data Registers: Offset and bit
#define PORT_IN   0
#define PORT_OUT  1
#define PORT_ALT1 2
#define PORT_ALT2 3
static int pinToRegData[] = { 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, //  0.. 7 XIO 0-7 no MMAP access (I2C proxy)
	0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, //  8..15 CSID0-7 (PE 4..11)
	0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, // 16..23 LCD3..(PD-3..7,10..12)
	0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, 0x7c, // 24..31 LCD13.. (PD-13..15,18..22)
	0x7c, 0x7c, 0x7c, 0x7c, 0x7c,   -1,   -1,   -1, // 32..36 LCD23.. (PD-23,24,26,27,25)
};
static int pinToRegDBit[] = { 
     0,  0,  0,  0,  0,  0,  0,  0, // no mmap access for XIO 0-7
     4,  5,  6,  7,  8,  9, 10, 11, // 8..15   (PE-4..11)
     3,  4,  5,  6,  7, 10, 11, 12, // 16..23  (PD-3..7,10..12)
    13, 14, 15, 18, 19, 20, 21, 22, // 24..31  (PD-13..15,18..22)
    23, 24, 26, 27, 25,  0,  0,  0, // 32..36  (PD-23,24,26,27,25)
};
//********************************************************************
// Port Pullup/Down resistor
#define PULL_OFF  0
#define PULL_UP   1
#define PULL_DOWN 2
//#define PULL_RESERVED 3

static int pinToPullUpDown[] = { 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // no MMAP access for XIO 0-7 (I2C proxy)
	0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, // CSID0-7 (PE 4..11)
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, // 16..23 (PD-3..7,10..12)
	0x88, 0x88, 0x88, 0x8C, 0x8C, 0x8C, 0x8C, 0x8C, // 24..31 (PD-13..15,18..22)
	0x8C, 0x8C, 0x8C, 0x8C, 0x8C,   -1,   -1,   -1, // 32..36 (PD-23,24,26,27,25)
};
static int pinToPullBits[] = { 
	  -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1, // no MMAP access for XIO 0-7 (I2C proxy)
	   8,   10,   12,   14,   16,   18,   20,   22, // CSID0-7 (PE 4..11)
	   6,    8,   10,   12,   14,   20,   22,   24, // 16..23 (PD-3..7,10..12)
	  26,   28,   30,    4,    6,    8,   10,   12, // 24..31 (PD-13..15,18..22)
	  14,   16,   20,   22,   18,   -1,   -1,   -1, // 32..36 (PD-23,24,26,27,25)
};

static int sysFds[64] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

#ifndef __FreeBSD__
/* SPI Bus Parameters */
static uint8_t     spiMode   = 0;
static uint8_t     spiBPW    = 8;
static uint16_t    spiDelay  = 0;
static uint32_t    spiSpeeds[4];
static int         spiFds[4];
#endif

static volatile uint32_t *gpio;

static unsigned int memReadl(uint32_t addr) {
	uint32_t val = 0;
	uint32_t mmap_base = (addr & ~PAGE_MASK);
	uint32_t mmap_seek = ((addr - mmap_base) >> 2);
	val = *((volatile uint32_t *) (gpio + mmap_seek));
	//wiringXLog(LOG_DEBUG, "chip->memRead: %x = %x", (gpio + mmap_seek), val);
	return val;
}

static void memWritel(uint32_t addr, uint32_t val) {
	uint32_t mmap_base = (addr & ~PAGE_MASK);
	uint32_t mmap_seek = ((addr - mmap_base) >> 2);
	*(gpio + mmap_seek) = ((volatile uint32_t) val);
	//wiringXLog(LOG_DEBUG, "chip->memWrite: %x = %x", (gpio + mmap_seek), val);
}


int chipValidGPIO(int pin) {
	if(pin >= 0 && pin < NUM_PINS && pinToGpio[pin] != -1) {
		return 0;
	}
	return -1;
}

static int changeOwner(char *file) {
	uid_t uid = getuid();
	uid_t gid = getgid();

	if(chown(file, uid, gid) != 0) {
		if(errno == ENOENT)	{
			wiringXLog(LOG_ERR, "chip->changeOwner: File not present: %s", file);
			return -1;
		} else {
			wiringXLog(LOG_ERR, "chip->changeOwner: Unable to change ownership of %s: %s", file, strerror (errno));
			return -1;
		}
	}
	return 0;
}

static int identify(void) {
	FILE *cpuFd;
	char line[120], revision[120], hardware[120], name[120];

	if((cpuFd = fopen("/proc/cpuinfo", "r")) == NULL) {
		wiringXLog(LOG_ERR, "chip->identify: Unable open /proc/cpuinfo");
		return -1;
	}

	while(fgets(line, 120, cpuFd) != NULL) {
		if(strncmp(line, "Revision", 8) == 0) {
			strcpy(revision, line);
		}
		if(strncmp(line, "Hardware", 8) == 0) {
			strcpy(hardware, line);
		}
	}

	fclose(cpuFd);

	sscanf(hardware, "Hardware%*[ \t]:%*[ ]%[a-zA-Z0-9 ./()]%*[\n]", name);

	if(strstr(name, "Allwinner sun4i/sun5i Families") != NULL) {
		return 0;
	} else {
		return -1;
	}
}

/*
 * setup mmap access
 *
 *******************************************************************************/
static int setup(void) {
	int fd;
	// TODO: test memory mapping - not fully implemented 
	if((fd = open("/dev/mem", O_RDWR | O_SYNC )) < 0) {
		wiringXLog(LOG_ERR, "chip->setup: Unable to open /dev/mem");
		return -1;
	}
	off_t addr = (GPIO_BASE & ~PAGE_MASK);
	gpio = (volatile uint32_t *)mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, addr);

	if((int32_t)gpio == -1) {
		wiringXLog(LOG_ERR, "chip->setup: mmap (GPIO) failed");
		return -1;
	} else
		wiringXLog(LOG_DEBUG, "chip->setup: GPIO at %x", gpio);

	return 0;
}

/* 
 * chipDigitalRead
 * 
 ***************************************************************************/
static int chipDigitalRead(int pin) {
  
	if (pinModes[pin] == SYS) {
		char c;
		if (pin == -1) {
			wiringXLog(LOG_ERR, "Invalid pin input %d", pin);
			return LOW ;
		}
		lseek(sysFds[pin], 0L, SEEK_SET) ;
		read (sysFds[pin], &c, 1) ;
		return (c == '0') ? LOW : HIGH ;

	} else if (pinModes[pin] == INPUT ) {
		// mmap access
		uint32_t physaddr = (uint32_t) (GPIO_OFFSET + pinToRegData[pin]);
        uint32_t val = memReadl(physaddr);
		return  (val >> pinToRegDBit[pin]) & 1; // single bit LOW | HIGH
	}
	wiringXLog(LOG_ERR, "Invalid input for pin %d", pin);
	return LOW;
}


/* 
 * chipDigitalWrite
 * 
 ***************************************************************************/
static int chipDigitalWrite(int pin, int value) {

	if (pinModes[pin] == SYS ) {
		if (sysFds[pin] != -1)
		{
			if (value == LOW)
				write (sysFds[pin], "0", 1) ;
			else
				write (sysFds[pin], "1", 1) ;   
			return 0;
		}
	} else if (pinModes[pin] == OUTPUT ) {
		// mmap access
		uint32_t physaddr = (uint32_t)(GPIO_OFFSET + pinToRegData[pin]);
        uint32_t regval = memReadl(physaddr);
        uint32_t nval;
		if(value == LOW) {
			nval = regval & ~(1 << pinToRegDBit[pin]);
		} else {
			nval = regval | (1 << pinToRegDBit[pin]);
		}
		memWritel(physaddr, nval);
		wiringXLog(LOG_DEBUG, "write pin %d, addr %04x, bit %d, val %08x -> %08x = %08x", 
				   pin, pinToRegData[pin], pinToRegDBit[pin], regval, nval, (uint32_t) memReadl(physaddr));
		return 0;
	}
	wiringXLog(LOG_ERR, "Invalid output for pin %d", pin);
	return -1;
}

/*
 * enableSysPin:
 *	
 *********************************************************************************
 */
static int enableSysPin(int pin) {
	
	//if (pinModes[pin] == SYS ) {
		// must use sysfs, workaround for OUTPUT
		char num[5];
		int fd = open("/sys/class/gpio/export", O_WRONLY, 0660);
		if (fd != -1) {
			sprintf(&num[0], "%d", pinToGpio[pin]);
			write (fd, &num, strlen(num));
			close (fd);
			return TRUE;
		} else {
			// alread enabled?
			wiringXLog(LOG_ERR, "enableSysPin: %d, %s", pin, strerror(errno));
		}
		return FALSE;
	//} else {
		// use mmap access
	//}
}

/*
 * chipPinModeMem
 *
 *********************************************************************/
static int chipPinModeMem(int pin, int mode) {
	// pin is the absolute number of a pin (0-36), regardless of the port

	uint32_t regaddr = (uint32_t) (GPIO_OFFSET + pinToRegMode[pin]);

	int bit = pinToRegMBit[pin]; 
	uint32_t val = memReadl(regaddr);
	// remove current 3 bits
	uint32_t nval = val & ~ ((uint32_t) (7 << bit));
	// 000 = Input, 001 = Output, others for special functions
	if (mode == OUTPUT) 
		nval = nval | ((uint32_t)(1 << bit));
		
	memWritel(regaddr, nval);
    uint32_t rval = memReadl(regaddr);
	wiringXLog(LOG_DEBUG, "ControlReg: %x , bit %d = %08x -> %08x = %08x", regaddr, bit, val, nval, rval);
	if (nval != rval) {
	    enableSysPin(pin);
		wiringXLog(LOG_DEBUG, "ControlReg failed: using file system");
	}
	return 0;
}

/*
 * setPinSysPath
 *  len values: 35 ("/sys/class/gpio/gpio123/active_low\0")
 *
 *******************************************************************************/
static int setPinSysPath(char *path, int len, int pin, const char *elem) {
  if (pin < 0 && pin >= NUM_PINS) 
    return FALSE;

  snprintf(path, len, "/sys/class/gpio/gpio%d/%s", pinToGpio[pin], elem);
  return TRUE;
}


/*
 * open SysFd (but access is still too slow)
 *
 ******************************************************************************/
static void openSysFd(int pin, int mode) {
	//  use sysfs
	char path[35];
	if (setPinSysPath(path, 35, pin, "value")) {
		// must not be set yet
		if (sysFds[pin] == -1) {
			delayMicroseconds(10); // let system perform internal update
			if (mode == OUTPUT) 
				sysFds[pin] = open(path, O_RDWR | O_CREAT, 0660) ;
			else
				sysFds[pin] = open(path, O_RDONLY) ;

			if (sysFds[pin] == -1)
				wiringXLog(LOG_ERR, "openSysFd: Pin %d, %s", pin, strerror(errno));
		}
		else {
			wiringXLog(LOG_WARNING, "Pin %d already in use", pin);
		}
	}
}

/*
 * pinMode:
 *	set pin for INPUT or OUTPUT
 ******************************************************************************
 */
static int chipPinMode (int pin, int mode)
{
	//wiringXLog(LOG_DEBUG, "pinMode: %d -> %s", pin, mode == OUTPUT ? "out" : "in");
	if (pinToRegMode[pin] == -1) {
	//if ( 1 == 1) {
		// must use sysfs
		char path[35];
		if (setPinSysPath(path, 35, pin, "direction")) {
			enableSysPin(pin);
			int fd = open(path, O_WRONLY ); 
			if (fd != -1) {
				if (mode == INPUT) {
					write (fd, "in\n", 3);
					openSysFd(pin, FALSE);
				} 
				else {
					write (fd, "out\n", 4);
					openSysFd(pin, TRUE);
				}
				close(fd);
				pinModes[pin] = SYS;
			} else {
				wiringXLog(LOG_ERR, "pinMode: %d -> %d, %s", pin, mode, strerror(errno));
			}
		} 
		if (pinToRegMode[pin] != -1) {
			pinModes[pin] = mode;
			wiringXLog(LOG_DEBUG, "pinMode: %d -> %s, MMAPed access forced", pin, mode == 0 ? "in" : "out");		
		}
	}
	else {
		// set direction via control register
		chipPinModeMem(pin, mode);
		pinModes[pin] = mode;
		wiringXLog(LOG_DEBUG, "pinMode: %d -> %s, MMAPed", pin, mode == 0 ? "in" : "out");		
	}
}


static int chipISR(int pin, int mode) {
	int i = 0, fd = 0, match = 0, count = 0;
	const char *sMode = NULL;
	char edgePath[35], valuePath[35], directionPath[35];
	char c, line[120];
	FILE *f = NULL;

	if(chipValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "chip->isr: Invalid pin number %d", pin);
		return -1;
	}	
	if (setPinSysPath(edgePath, 35, pin, "edge")) {
		setPinSysPath(valuePath, 35, pin, "value");
		setPinSysPath(directionPath, 35, pin, "direction");

		pinModes[pin] = SYS;

		if(mode == INT_EDGE_FALLING) {
			sMode = "falling" ;
		} else if(mode == INT_EDGE_RISING) {
			sMode = "rising" ;
		} else if(mode == INT_EDGE_BOTH) {
			sMode = "both";
		} else if(mode == INT_EDGE_NONE) {
			sMode = "none";
		} else {
			wiringXLog(LOG_ERR, "chip->isr: Invalid mode. Should be INT_EDGE_BOTH, INT_EDGE_RISING, or INT_EDGE_FALLING");
			return -1;
		}

		fd = open(valuePath, O_RDWR);

		if(fd < 0) {
			if((f = fopen("/sys/class/gpio/export", "w")) == NULL) {
				wiringXLog(LOG_ERR, "chip->isr: Unable to open GPIO export interface: %s", strerror(errno));
				return -1;
			}
			fprintf(f, "%d\n", pinToGpio[pin]);
			fclose(f);
		}

		if((f = fopen(directionPath, "w")) == NULL) {
			wiringXLog(LOG_ERR, "chip->isr: Unable to open GPIO direction interface for pin %d: %s", pin, strerror(errno));
			return -1;
		}

		fprintf(f, "in\n");
		fclose(f);

		if((f = fopen(edgePath, "w")) == NULL) {
			wiringXLog(LOG_ERR, "chip->isr: Unable to open GPIO edge interface for pin %d: %s", pin, strerror(errno));
			return -1;
		}

		if(strcasecmp(sMode, "none") == 0) {
			fprintf(f, "none\n");
		} else if(strcasecmp(sMode, "rising") == 0) {
			fprintf(f, "rising\n");
		} else if(strcasecmp(sMode, "falling") == 0) {
			fprintf(f, "falling\n");
		} else if(strcasecmp (sMode, "both") == 0) {
			fprintf(f, "both\n");
		} else {
			wiringXLog(LOG_ERR, "chip->isr: Invalid mode: %s. Should be rising, falling or both", sMode);
			return -1;
		}
		fclose(f);

		if((f = fopen(edgePath, "r")) == NULL) {
			wiringXLog(LOG_ERR, "chip->isr: Unable to open GPIO edge interface for pin %d: %s", pin, strerror(errno));
			return -1;
		}

		match = 0;
		while(fgets(line, 120, f) != NULL) {
			if(strstr(line, sMode) != NULL) {
				match = 1;
				break;
			}
		}
		fclose(f);

		if(match == 0) {
			wiringXLog(LOG_ERR, "chip->isr: Failed to set interrupt edge to %s", sMode);
			return -1;	
		}

		if (sysFds[pin] == -1) {
			if((sysFds[pin] = open(valuePath, O_RDONLY)) < 0) {
				wiringXLog(LOG_ERR, "chip->isr: Unable to open GPIO value interface: %s", strerror(errno));
				return -1;
			}
			changeOwner(valuePath);
		} 

		changeOwner(edgePath);

		ioctl(fd, FIONREAD, &count);
		for(i=0; i<count; ++i) {
			read(fd, &c, 1);
		}
		close(fd);
	}	
	return 0;
}

static int chipWaitForInterrupt(int pin, int ms) {
	int x = 0;
	uint8_t c = 0;
	struct pollfd polls;

	if(chipValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "chip->waitForInterrupt: Invalid pin number %d", pin);
		return -1;
	}

	if(pinModes[pin] != SYS) {
		wiringXLog(LOG_ERR, "chip->waitForInterrupt: Trying to read from pin %d, but it's not configured as interrupt", pin);
		return -1;
	}

	if(sysFds[pin] == -1) {
		wiringXLog(LOG_ERR, "chip->waitForInterrupt: GPIO %d not set as interrupt", pin);
		return -1;
	}

	polls.fd = sysFds[pin];
	polls.events = POLLPRI;

	(void)read(sysFds[pin], &c, 1);
	lseek(sysFds[pin], 0, SEEK_SET);		
	
	x = poll(&polls, 1, ms);

	/* Don't react to signals */
	if(x == -1 && errno == EINTR) {
		x = 0;
	}

	return x;
}

static int chipGC(void) {
	int pin = 0, fd = 0;
	char path[35];
	FILE *f = NULL;

	for(pin=0; pin<NUM_PINS; pin++) {
		if(pinModes[pin] == OUTPUT) {
			// default INPUT
			pinMode(pin, INPUT);
		} else if(pinModes[pin] == SYS) {
			// Pins accessed via sysfs
			setPinSysPath(path, 35, pin, "value");
			if((fd = open(path, O_RDWR)) > 0) {
				if((f = fopen("/sys/class/gpio/unexport", "w")) == NULL) {
					wiringXLog(LOG_ERR, "chip->gc: Unable to open GPIO unexport interface: %s", strerror(errno));
				}

				fprintf(f, "%d\n", pinToGpio[pin]);
				fclose(f);
				close(fd);
			}
		}
		if(sysFds[pin] > 0) {
			close(sysFds[pin]);
			sysFds[pin] = -1;
		}
	}

	if(gpio) {
		munmap((void *)gpio, PAGE_SIZE);
	}
	return 0;
}

#ifndef __FreeBSD__
static int chipI2CRead(int fd) {
	return i2c_smbus_read_byte(fd);
}

static int chipI2CReadReg8(int fd, int reg) {
	return i2c_smbus_read_byte_data(fd, reg);
}

static int chipI2CReadReg16(int fd, int reg) {
	return i2c_smbus_read_word_data(fd, reg);
}

static int chipI2CWrite(int fd, int data) {
	return i2c_smbus_write_byte(fd, data);
}

static int chipI2CWriteReg8(int fd, int reg, int data) {
	return i2c_smbus_write_byte_data(fd, reg, data);
}

static int chipI2CWriteReg16(int fd, int reg, int data) {
	return i2c_smbus_write_word_data(fd, reg, data);
}

static int chipI2CSetup(int devId) {
	int fd = 0;
	const char *device = channelToDevice[devId & 3];

	if((fd = open(device, O_RDWR)) < 0) {
		wiringXLog(LOG_ERR, "chip->I2CSetup: Unable to open %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, devId) < 0) {
		wiringXLog(LOG_ERR, "chip->I2CSetup: Unable to set %s to slave mode: %s", device, strerror(errno));
		return -1;
	}

	return fd;
}

int chipSPIGetFd(int channel) {
	return spiFds[channel & 3];
}

int chipSPIDataRW(int channel, unsigned char *data, int len) {
	struct spi_ioc_transfer spi;
	memset(&spi, 0, sizeof(spi)); // found at http://www.raspberrypi.org/forums/viewtopic.php?p=680665#p680665
	channel &= 3;

	spi.tx_buf = (unsigned long)data;
	spi.rx_buf = (unsigned long)data;
	spi.len = len;
	spi.delay_usecs = spiDelay;
	spi.speed_hz = spiSpeeds[channel];
	spi.bits_per_word = spiBPW;
#ifdef SPI_IOC_WR_MODE32
	spi.tx_nbits = 0;
#endif
#ifdef SPI_IOC_RD_MODE32
	spi.rx_nbits = 0;
#endif

	if(ioctl(spiFds[channel], SPI_IOC_MESSAGE(1), &spi) < 0) {
		wiringXLog(LOG_ERR, "chip->SPIDataRW: Unable to read/write from channel %d: %s", channel, strerror(errno));
		return -1;
	}
	return 0;
}

int chipSPISetup(int channel, int speed) {
	int fd;
	const char *device = channelToDevice[channel & 3];

	if((fd = open(device, O_RDWR)) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to open device %s: %s", device, strerror(errno));
		return -1;
	}

	spiSpeeds[channel & 3] = speed;
	spiFds[channel & 3] = fd;

	if(ioctl(fd, SPI_IOC_WR_MODE, &spiMode) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to set write mode for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_MODE, &spiMode) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to set read mode for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to set write bits_per_word for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spiBPW) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to set read bits_per_word for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to set write max_speed for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		wiringXLog(LOG_ERR, "chip->SPISetup: Unable to set read max_speed for device %s: %s", device, strerror(errno));
		return -1;
	}

	return fd;
}
#endif

void chipInit(void) {

	memset(pinModes, -1, NUM_PINS);

	platform_register(&chip, "C.H.I.P.");
	chip->setup = &setup;
	chip->pinMode = &chipPinMode;
	chip->digitalWrite = &chipDigitalWrite;
	chip->digitalRead = &chipDigitalRead;
	chip->identify = &identify;
	chip->isr = &chipISR;
	chip->waitForInterrupt = &chipWaitForInterrupt;
#ifndef __FreeBSD__
/*	chip->I2CRead = &chipI2CRead;
	chip->I2CReadReg8 = &chipI2CReadReg8;
	chip->I2CReadReg16 = &chipI2CReadReg16;
	chip->I2CWrite = &chipI2CWrite;
	chip->I2CWriteReg8 = &chipI2CWriteReg8;
	chip->I2CWriteReg16 = &chipI2CWriteReg16;
	chip->I2CSetup = &chipI2CSetup;
	chip->SPIGetFd = &chipSPIGetFd;
	chip->SPIDataRW = &chipSPIDataRW;
	chip->SPISetup = &chipSPISetup;
*/
#endif
	chip->gc = &chipGC;
	chip->validGPIO = &chipValidGPIO;
}
