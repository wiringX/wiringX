/*
	Copyright (c) 2016 Brian Kim <brian.kim@hardkernel.com>	 
	Copyright (c) 2014 CurlyMo <curlymoo1@gmail.com>
	2012 Gordon Henderson

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
#include "odroid.h"

#define OD_MODEL_ODROIDC           6
#define OD_MODEL_ODROIDXU_34       7
#define OD_MODEL_ODROIDC2          8

// For ODROID-C1
#define C1_AinNode0                "/sys/class/saradc/saradc_ch0"
#define C1_AinNode1                "/sys/class/saradc/saradc_ch1"
#define C1_VERSION                 1

#define C1_GPIOY_PIN_START         80
#define C1_GPIOY_PIN_END           96
#define C1_GPIOX_PIN_START         97
#define C1_GPIOX_PIN_END           118

#define C1_GPIOY_OUTP_REG_OFFSET   0x10
#define C1_GPIOY_INP_REG_OFFSET    0x11
#define C1_GPIOX_FSEL_REG_OFFSET   0x0C
#define C1_GPIOX_OUTP_REG_OFFSET   0x0D
#define C1_GPIOX_INP_REG_OFFSET    0x0E
#define C1_GPIOY_FSEL_REG_OFFSET   0x0F

// For ODROID-C2
#define C2_AinNode0                "/sys/class/saradc/ch0"
#define C2_AinNode1                "/sys/class/saradc/ch1"
#define C2_VERSION                 1

#define C2_GPIO_PIN_BASE           136
#define C2_GPIOY_PIN_START         (C2_GPIO_PIN_BASE + 75)
#define C2_GPIOY_PIN_END           (C2_GPIO_PIN_BASE + 91)
#define C2_GPIOX_PIN_START         (C2_GPIO_PIN_BASE + 92)
#define C2_GPIOX_PIN_END           (C2_GPIO_PIN_BASE + 114)

#define C2_GPIOX_FSEL_REG_OFFSET   0x118
#define C2_GPIOX_OUTP_REG_OFFSET   0x119
#define C2_GPIOX_INP_REG_OFFSET    0x11A
#define C2_GPIOY_FSEL_REG_OFFSET   0x10F
#define C2_GPIOY_OUTP_REG_OFFSET   0x110
#define C2_GPIOY_INP_REG_OFFSET    0x111

// For ODROID-XU3/4
#define XU_AinNode0                "/sys/devices/12d10000.adc/iio:device0/in_voltage0_raw"
#define XU_AinNode1                "/sys/devices/12d10000.adc/iio:device0/in_voltage3_raw"
#define XU_VERSION                 1

#define ODROIDXU_GPX_BASE          0x13400000  // GPX0,1,2,3
#define ODROIDXU_GPA_BASE          0x14010000  // GPA0,1,2, GPB0,1,2,3,4
#define GPIO_X1_START              16
#define GPIO_X1_CON_OFFSET         0x0C20
#define GPIO_X1_DAT_OFFSET         0x0C24
#define GPIO_X1_END                23

#define GPIO_X2_START              24
#define GPIO_X2_CON_OFFSET         0x0C40
#define GPIO_X2_DAT_OFFSET         0x0C44
#define GPIO_X2_PUD_OFFSET         0x0C48
#define GPIO_X2_END                31

#define GPIO_X3_START              32
#define GPIO_X3_CON_OFFSET         0x0C60
#define GPIO_X3_DAT_OFFSET         0x0C64
#define GPIO_X3_PUD_OFFSET         0x0C68
#define GPIO_X3_END                39

#define GPIO_A0_START              171
#define GPIO_A0_CON_OFFSET         0x0000
#define GPIO_A0_DAT_OFFSET         0x0004
#define GPIO_A0_PUD_OFFSET         0x0008
#define GPIO_A0_END                178

#define GPIO_A2_START              185
#define GPIO_A2_CON_OFFSET         0x0040
#define GPIO_A2_DAT_OFFSET         0x0044
#define GPIO_A2_PUD_OFFSET         0x0048
#define GPIO_A2_END                192

#define GPIO_B3_START              207
#define GPIO_B3_CON_OFFSET         0x00C0
#define GPIO_B3_DAT_OFFSET         0x00C4
#define GPIO_B3_PUD_OFFSET         0x00C8
#define GPIO_B3_END                214

#define NUM_PINS                   32
#define	BLOCK_SIZE                 (4*1024)

static volatile uint32_t ODROID_GPIO_BASE = 0;
static uint32_t ODROID_GPIO_MASK = 0;
static uint32_t GPIOX_PIN_START = 0;
static uint32_t GPIOY_PIN_START = 0;
static uint32_t GPIOX_PIN_END = 0;
static uint32_t GPIOY_PIN_END = 0;

static uint32_t GPIOX_OUTP_REG_OFFSET = 0;
static uint32_t GPIOY_OUTP_REG_OFFSET = 0;
static uint32_t GPIOX_INP_REG_OFFSET = 0;
static uint32_t GPIOY_INP_REG_OFFSET = 0;
static uint32_t GPIOX_FSEL_REG_OFFSET = 0;
static uint32_t GPIOY_FSEL_REG_OFFSET = 0;

static const char *AinNode0 = NULL;
static const char *AinNode1 = NULL;

static int odModel = 0;
static volatile uint32_t *gpio;
static int pinModes[NUM_PINS];

static uint8_t gpioToShift[] = {
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
	0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
};

static uint8_t gpioToGPFSEL[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

//
// pinToGpio:
//	Take a Wiring pin (0 through X) and re-map it to the ODROID_GPIO pin
//
static int *pinToGpio;

static int pinToGpio_C1[64] = {
    88,  87, 116, 115, 104, 102, 103,  83, // 0..7
    -1,  -1, 117, 118, 107, 106, 105,  -1, // 8..16
    -1,  -1,  -1,  -1,  -1, 101, 100, 108, // 16..23
    97,  -1,  99,  98,  -1,  -1,  -1,  -1, // 24..31
// Padding:
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// ... 47
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// ... 63
};

static int pinToGpio_C2[64] = {
   247, 238, 239, 237, 236, 233, 231, 249, // 0..7
    -1,  -1, 229, 225, 235, 232, 230,  -1, // 8..15
    -1,  -1,  -1,  -1,  -1, 228, 219, 234, // 16..23
   214,  -1, 224, 218,  -1,  -1,  -1,  -1, // 24..31
// Padding:
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// ... 47
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// ... 63
};

static int pinToGpio_XU[64] = {
   174, 173,    //  0 |  1 : GPA0.3(UART_0.CTSN), GPA0.2(UART_0.RTSN)
    21,  22,    //  2 |  3 : GPX1.5, GPX1.6
    19,  23,    //  4 |  5 : GPX1.3, GPX1.7
    24,  18,    //  6 |  7 : GPX2.0, GPX1.2

   209, 210,    //  8 |  9 : GPB3.2(I2C_1.SDA), GPB3.3(I2C_1.SCL)
   190,  25,    // 10 | 11 : GPA2.5(SPI_1.CSN), GPX2.1
   192, 191,    // 12 | 13 : GPA2.7(SPI_1.MOSI), GPA2.6(SPI_1.MISO)
   189, 172,    // 14 | 15 : GPA2.4(SPI_1.SCLK), GPA0.1(UART_0.TXD)
   171,  -1,    // 16 | 17 : GPA0.0(UART_0.RXD),
    -1,  -1,    // 18 | 19
    -1,  28,    // 20 | 21 :  , GPX2.4
    30,  31,    // 22 | 23 : GPX2.6, GPX2.7
    -1,  -1,    // 24 | 25   PWR_ON(INPUT), ADC_0.AIN0
    29,  33,    // 26 | 27 : GPX2.5, GPX3.1
    -1,  -1,    // 28 | 29 : REF1.8V OUT, ADC_0.AIN3
   187, 188,    // 30 | 31 : GPA2.2(I2C_5.SDA), GPA2.3(I2C_5.SCL)

    // Padding:
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 32...47
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 48...63
};

//
// physToGpio:
//	Take a physical pin (1 through 40) and re-map it to the ODROID_GPIO pin
static int *physToGpio;

static int physToGpio_C1[64] =
{
  -1,       // 0
  -1,  -1,  // 1, 2
  -1,  -1,
  -1,  -1,
  83,  -1,
  -1,  -1,
  88,  87,
 116,  -1,
 115, 104,
  -1, 102,
 107,  -1,
 106, 103,
 105, 117,
  -1, 118, // 25, 26

  -1,  -1,
 101,  -1,
 100,  99,
 108,  -1,
  97,  98,
  -1,  -1,
  -1,  -1, // 39, 40

  // Not used
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
};

static int physToGpio_C2[64] =
{
  -1,      // 0
  -1,  -1, // 1, 2
  -1,  -1,
  -1,  -1,
 249,  -1,
  -1,  -1,
 247, 238,
 239,  -1,
 237, 236,
  -1, 233,
 235,  -1,
 232, 231,
 230, 229,
  -1, 225, // 25, 26

  -1,  -1,
 228,  -1,
 219, 224,
 234,  -1,
 214, 218,
  -1,  -1,
  -1,  -1, // 39, 40

  // Not used
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
};

static int physToGpio_XU[64] =
{
    -1,      //  0
    -1,  -1, //  1 |  2 : 3.3V, 5.0V
   209,  -1, //  3 |  4 : GPB3.2(I2C_1.SDA), 5.0V
   210,  -1, //  5 |  6 : GPB3.3(I2C_1.SCL), GND
    18, 172, //  7 |  8 : GPX1.2, GPA0.1(UART_0.TXD)
    -1, 171, //  9 | 10 : GND, GPA0.0(UART_0.RXD)
   174, 173, // 11 | 12 : GPA0.3(UART_0.CTSN), GPA0.2(UART_0.RTSN)
    21,  -1, // 13 | 14 : GPX1.5, GND
    22,  19, // 15 | 16 : GPX1.6, GPX1.3
    -1,  23, // 17 | 18 : 3.3V, GPX1.7
   192,  -1, // 19 | 20 : GPA2.7(SPI_1.MOSI), GND
   191,  24, // 21 | 22 : GPA2.6(SPI_1.MISO), GPX2.0
   189, 190, // 23 | 24 : GPA2.4(SPI_1.SCLK), GPA2.5(SPI_1.CSN)
    -1,  25, // 25 | 26 : GND, GPX2.1
   187, 188, // 27 | 28 : GPA2.2(I2C_5.SDA), GPA2.4(I2C_5.SCL)
    28,  -1, // 29 | 30 : GPX2.4, GND
    30,  29, // 31 | 32 : GPX2.6, GPX2.5
    31,  -1, // 33 | 34 : GPX2.7, GND
    -1,  33, // 35 | 36 : PWR_ON(INPUT), GPX3.1
    -1,  -1, // 37 | 38 : ADC_0.AIN0, 1.8V REF OUT
    -1,  -1, // 39 | 40 : GND, AADC_0.AIN3

    // Not used
    -1, -1, -1, -1, -1, -1, -1, -1, // 41...48
    -1, -1, -1, -1, -1, -1, -1, -1, // 49...56
    -1, -1, -1, -1, -1, -1, -1      // 57...63
};

static int sysFds[64] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

/* SPI Bus Parameters */
#ifndef __FreeBSD__
static uint8_t     spiMode   = 0;
static uint8_t     spiBPW    = 8;
static uint16_t    spiDelay  = 0;
static uint32_t    spiSpeeds[2];
static int         spiFds[2];
#endif

static int adcFds[2] = {
	-1, -1,
};

//
// offset to the GPIO Set regsiter
//
static int gpioToGPSETReg(int pin) {
	if (odModel == OD_MODEL_ODROIDXU_34) {
		switch(pin) {
			case    GPIO_X1_START...GPIO_X1_END:
				return  (GPIO_X1_DAT_OFFSET >> 2);
			case    GPIO_X2_START...GPIO_X2_END:
				return  (GPIO_X2_DAT_OFFSET >> 2);
			case    GPIO_X3_START...GPIO_X3_END:
				return  (GPIO_X3_DAT_OFFSET >> 2);
			case    GPIO_A0_START...GPIO_A0_END:
				return  (GPIO_A0_DAT_OFFSET >> 2);
			case    GPIO_A2_START...GPIO_A2_END:
				return  (GPIO_A2_DAT_OFFSET >> 2);
			case    GPIO_B3_START...GPIO_B3_END:
				return  (GPIO_B3_DAT_OFFSET >> 2);
			default:
				break;
		}
	} else {
		if(pin >= GPIOX_PIN_START && pin <= GPIOX_PIN_END) {
			return GPIOX_OUTP_REG_OFFSET;
		}
		if(pin >= GPIOY_PIN_START && pin <= GPIOY_PIN_END) {
			return GPIOY_OUTP_REG_OFFSET;
		}
	}
	return -1;
}

//
// offset to the GPIO Input regsiter
//
static int  gpioToGPLEVReg (int pin) {
	if (odModel == OD_MODEL_ODROIDXU_34) {
		switch(pin) {
			case    GPIO_X1_START...GPIO_X1_END:
				return  (GPIO_X1_DAT_OFFSET >> 2);
			case    GPIO_X2_START...GPIO_X2_END:
				return  (GPIO_X2_DAT_OFFSET >> 2);
			case    GPIO_X3_START...GPIO_X3_END:
				return  (GPIO_X3_DAT_OFFSET >> 2);
			case    GPIO_A0_START...GPIO_A0_END:
				return  (GPIO_A0_DAT_OFFSET >> 2);
			case    GPIO_A2_START...GPIO_A2_END:
				return  (GPIO_A2_DAT_OFFSET >> 2);
			case    GPIO_B3_START...GPIO_B3_END:
				return  (GPIO_B3_DAT_OFFSET >> 2);
			default:
				break;
		}
	} else {
		if(pin >= GPIOX_PIN_START && pin <= GPIOX_PIN_END) {
			return GPIOX_INP_REG_OFFSET;
		}
		if(pin >= GPIOY_PIN_START && pin <= GPIOY_PIN_END) {
			return GPIOY_INP_REG_OFFSET;
		}
	}
	return -1;
}

//
// offset to the GPIO bit
//
static int gpioToShiftReg(int pin) {
	if (odModel == OD_MODEL_ODROIDXU_34) {
		switch(pin) {
			case    GPIO_X1_START...GPIO_X1_END:
				return  (pin - GPIO_X1_START);
			case    GPIO_X2_START...GPIO_X2_END:
				return  (pin - GPIO_X2_START);
			case    GPIO_X3_START...GPIO_X3_END:
				return  (pin - GPIO_X3_START);
			case    GPIO_A0_START...GPIO_A0_END:
				return  (pin - GPIO_A0_START);
			case    GPIO_A2_START...GPIO_A2_END:
				return  (pin - GPIO_A2_START);
			case    GPIO_B3_START...GPIO_B3_END:
				return  (pin - GPIO_B3_START);
			default:
				break;
		}
	} else {
		if(pin >= GPIOX_PIN_START && pin <= GPIOX_PIN_END) {
			return pin - GPIOX_PIN_START;
		}
		if(pin >= GPIOY_PIN_START && pin <= GPIOY_PIN_END) {
			return pin - GPIOY_PIN_START;
		}
	}

	return -1;
}

//
// offset to the GPIO Function register
//
static int gpioToGPFSELReg(int pin) {
	if (odModel == OD_MODEL_ODROIDXU_34) {
		switch(pin) {
			case    GPIO_X1_START...GPIO_X1_END:
				return  (GPIO_X1_CON_OFFSET >> 2);
			case    GPIO_X2_START...GPIO_X2_END:
				return  (GPIO_X2_CON_OFFSET >> 2);
			case    GPIO_X3_START...GPIO_X3_END:
				return  (GPIO_X3_CON_OFFSET >> 2);
			case    GPIO_A0_START...GPIO_A0_END:
				return  (GPIO_A0_CON_OFFSET >> 2);
			case    GPIO_A2_START...GPIO_A2_END:
				return  (GPIO_A2_CON_OFFSET >> 2);
			case    GPIO_B3_START...GPIO_B3_END:
				return  (GPIO_B3_CON_OFFSET >> 2);
			default:
				break;
		}
	} else {
		if(pin >= GPIOX_PIN_START && pin <= GPIOX_PIN_END) {
			return GPIOX_FSEL_REG_OFFSET;
		}
		if(pin >= GPIOY_PIN_START && pin <= GPIOY_PIN_END) {
			return GPIOY_FSEL_REG_OFFSET;
		}
	}

	return -1;
}

int odroidValidGPIO(int pin) {
	if(pinToGpio[pin] != -1) {
		return 0;
	}
	return -1;
}

static int changeOwner(char *file) {
	uid_t uid = getuid();
	uid_t gid = getgid();

	if(chown(file, uid, gid) != 0) {
		if(errno == ENOENT)	{
			wiringXLog(LOG_ERR, "odroid->changeOwner: File not present: %s", file);
			return -1;
		} else {
			wiringXLog(LOG_ERR, "odroid->changeOwner: Unable to change ownership of %s: %s", file, strerror (errno));
			return -1;
		}
	}

	return 0;
}

static int c1_init(void) {
	odModel = OD_MODEL_ODROIDC;
	ODROID_GPIO_BASE = 0xC1108000;
	ODROID_GPIO_MASK = 0xFFFFFF80;
	pinToGpio = pinToGpio_C1;
	physToGpio = physToGpio_C1;

	AinNode0 = C1_AinNode0;
	AinNode1 = C1_AinNode1;

	GPIOY_PIN_START	= C1_GPIOY_PIN_START;
	GPIOY_PIN_END   = C1_GPIOY_PIN_END;
	GPIOX_PIN_START = C1_GPIOX_PIN_START;
	GPIOX_PIN_END   = C1_GPIOX_PIN_END;

	GPIOX_INP_REG_OFFSET  = C1_GPIOX_INP_REG_OFFSET;
	GPIOY_INP_REG_OFFSET  = C1_GPIOY_INP_REG_OFFSET;
	GPIOX_OUTP_REG_OFFSET = C1_GPIOX_OUTP_REG_OFFSET;
	GPIOY_OUTP_REG_OFFSET = C1_GPIOY_OUTP_REG_OFFSET;
	GPIOX_FSEL_REG_OFFSET = C1_GPIOX_FSEL_REG_OFFSET;
	GPIOY_FSEL_REG_OFFSET = C1_GPIOY_FSEL_REG_OFFSET;

	return C1_VERSION;
}

static int c2_init(void) {
	odModel = OD_MODEL_ODROIDC2;
	ODROID_GPIO_BASE = 0xC8834000;
	ODROID_GPIO_MASK = 0xFFFFFF00;
	pinToGpio = pinToGpio_C2;
	physToGpio = physToGpio_C2;

	AinNode0 = C2_AinNode0;
	AinNode1 = C2_AinNode1;

	GPIOY_PIN_START	= C2_GPIOY_PIN_START;
	GPIOY_PIN_END   = C2_GPIOY_PIN_END;
	GPIOX_PIN_START = C2_GPIOX_PIN_START;
	GPIOX_PIN_END   = C2_GPIOX_PIN_END;

	GPIOX_INP_REG_OFFSET  = C2_GPIOX_INP_REG_OFFSET;
	GPIOY_INP_REG_OFFSET  = C2_GPIOY_INP_REG_OFFSET;
	GPIOX_OUTP_REG_OFFSET = C2_GPIOX_OUTP_REG_OFFSET;
	GPIOY_OUTP_REG_OFFSET = C2_GPIOY_OUTP_REG_OFFSET;
	GPIOX_FSEL_REG_OFFSET = C2_GPIOX_FSEL_REG_OFFSET;
	GPIOY_FSEL_REG_OFFSET = C2_GPIOY_FSEL_REG_OFFSET;

	return C2_VERSION;
}

static int xu_init(void) {
	odModel = OD_MODEL_ODROIDXU_34;
	ODROID_GPIO_MASK = 0xFFFFFF00;
	pinToGpio = pinToGpio_XU;
	physToGpio = physToGpio_XU;

	AinNode0 = XU_AinNode0;
	AinNode1 = XU_AinNode1;

	return XU_VERSION;
}

static int odBoardRev(void) {
	FILE *cpuFd;
	char line[120], revision[120], hardware[120], name[120];
	char *c;
	static int boardRev = -1;

	memset(line, '\0', 120);

	if((cpuFd = fopen("/proc/cpuinfo", "r")) == NULL) {
		wiringXLog(LOG_ERR, "odroid->identify: Unable open /proc/cpuinfo");
		goto err;
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

	if(strstr(name, "ODROID") != NULL) {
		if(boardRev != -1) {
			return boardRev;
		}

		if((cpuFd = fopen("/proc/cpuinfo", "r")) == NULL) {
			wiringXLog(LOG_ERR, "odroid->identify: Unable to open /proc/cpuinfo");
			goto err;
		}

		while(fgets(line, 120, cpuFd) != NULL) {
			if(strncmp(line, "Revision", 8) == 0) {
				break;
			}
		}

		fclose(cpuFd);

		if(strncmp(line, "Revision", 8) != 0) {
			wiringXLog(LOG_ERR, "odroid->identify: No \"Revision\" line");
			goto err;
		}

		for(c = &line[strlen(line) - 1] ; (*c == '\n') || (*c == '\r'); --c) {
			*c = 0;
		}

		for(c = line; *c; ++c) {
			if(isdigit(*c)) {
				break;
			}
		}

		if(!isdigit(*c)) {
			wiringXLog(LOG_ERR, "odroid->identify: No numeric revision string");
			goto err;
		}

		if(strlen(c) < 4) {
			wiringXLog(LOG_ERR, "odroid->identify: Bogus \"Revision\" line (too small)");
			goto err;
		}

		c = c + strlen(c) - 4;

		if (strcmp(c, "000a") == 0) {
			boardRev = c1_init();
		} else if (strcmp(c, "020b") == 0) {
			boardRev = c2_init();
		} else if (strcmp(c, "0100") == 0) {
			boardRev = xu_init();
		} else {
			wiringXLog(LOG_ERR, "odroid->identify: Unknown hardware");
			goto err;
		}
		return boardRev;
	}
err:
	return -1;
}

static int setup(void)	{
	int fd;
	int boardRev;
	void *pc;

	boardRev = odBoardRev();

	if((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
		wiringXLog(LOG_ERR, "odroid->setup: Unable to open /dev/mem");
		return -1;
	}

	if (odModel == OD_MODEL_ODROIDC || odModel == OD_MODEL_ODROIDC2) {
		pc = (void *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, ODROID_GPIO_BASE);
		if(pc == MAP_FAILED)
			goto mmap_err;
	} else if (odModel == OD_MODEL_ODROIDXU_34) {
		pc = (void *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, ODROIDXU_GPX_BASE);
		if(pc == MAP_FAILED)
			goto mmap_err;
		pc = (void *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, ODROIDXU_GPA_BASE);
		if(pc == MAP_FAILED)
			goto mmap_err;
	} else {
		wiringXLog(LOG_ERR, "odroid->setup: Unknown hardware");
		return -1;
	}

	adcFds[0] = open(AinNode0, O_RDONLY);
	adcFds[1] = open(AinNode1, O_RDONLY);

	gpio = (uint32_t *)pc;

	return 0;

mmap_err:
	wiringXLog(LOG_ERR, "odroid->setup: mmap (GPIO) failed");
	return -1;
}

static int odroidAnalogRead(int channel) {
	uint8_t value[5] = {0,};

	if (channel != 0 && channel != 1) {
		wiringXLog(LOG_ERR, "odroid->analogRead: %d channel can not support", channel);
		return -1;
	}

	lseek(adcFds[channel], 0L, SEEK_SET);
	if (read(adcFds[channel], &value[0], 4) == -1) {
		wiringXLog(LOG_ERR, "odroid->analogRead: read failed");
		return -1;
	}
	return atoi(value);
}

static int odroidDigitalRead(int pin) {
	if(pinModes[pin] != INPUT && pinModes[pin] != SYS) {
		wiringXLog(LOG_ERR, "odroid->digitalRead: Trying to write to pin %d, but it's not configured as input", pin);
		return -1;
	}

	if(odroidValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "odroid->digitalRead: Invalid pin number %d", pin);
		return -1;
	}	

	if((pin & ODROID_GPIO_MASK) == 0) {
		pin = pinToGpio[pin] ;

		 if ((*(gpio + gpioToGPLEVReg(pin)) & (1 << gpioToShiftReg(pin))) != 0) {
			return HIGH;
		} else {
			return LOW;
		}
	}
	return 0;
}

static int odroidDigitalWrite(int pin, int value) {
	if(pinModes[pin] != OUTPUT) {
		wiringXLog(LOG_ERR, "odroid->digitalWrite: Trying to write to pin %d, but it's not configured as output", pin);
		return -1;
	}

	if(odroidValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "odroid->digitalWrite: Invalid pin number %d", pin);
		return -1;
	}	

	if((pin & ODROID_GPIO_MASK) == 0) {
		pin = pinToGpio[pin] ;

		if(value == LOW)
			*(gpio + gpioToGPSETReg(pin)) &= ~(1 << gpioToShiftReg(pin));
		else
			*(gpio + gpioToGPSETReg(pin)) |=  (1 << gpioToShiftReg(pin));
	}
	return 0;
}

static int odroidPinMode(int pin, int mode) {
	int fSel, shift;

	if(odroidValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "odroid->pinMode: Invalid pin number %d", pin);
		return -1;
	}	

	if((pin & ODROID_GPIO_MASK) == 0) {
		pinModes[pin] = mode;
		pin = pinToGpio[pin];

		fSel = gpioToGPFSEL[pin];
		shift = gpioToShift[pin];

		if(mode == INPUT) {
			*(gpio + gpioToGPFSELReg(pin)) = (*(gpio + gpioToGPFSELReg(pin)) |  (1 << gpioToShiftReg(pin)));   
		} else if(mode == OUTPUT) {
			*(gpio + gpioToGPFSELReg(pin)) = (*(gpio + gpioToGPFSELReg(pin)) & ~(1 << gpioToShiftReg(pin)));
		}
	}
	return 0;
}

static int odroidISR(int pin, int mode) {
	int i = 0, fd = 0, match = 0, count = 0;
	const char *sMode = NULL;
	char path[64], c, line[120];
	FILE *f = NULL;
	const char *gpio_value_name = "/sys/class/gpio/gpio%d/value";
	const char *gpio_export_name = "/sys/class/gpio/export";
	const char *gpio_direction_name = "/sys/class/gpio/gpio%d/direction";
	const char *gpio_edge_name = "/sys/class/gpio/gpio%d/edge";

	if(odroidValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "odroid->isr: Invalid pin number %d", pin);
		return -1;
	}	

	pinModes[pin] = SYS;

	if(mode == INT_EDGE_FALLING) {
		sMode = "falling" ;
	} else if(mode == INT_EDGE_RISING) {
		sMode = "rising" ;
	} else if(mode == INT_EDGE_BOTH) {
		sMode = "both";
	} else {
		wiringXLog(LOG_ERR, "odroid->isr: Invalid mode. Should be INT_EDGE_BOTH, INT_EDGE_RISING, or INT_EDGE_FALLING");
		return -1;
	}

	if (odModel == OD_MODEL_ODROIDC || odModel == OD_MODEL_ODROIDC2) {
		gpio_value_name = "/sys/class/aml_gpio/gpio%d/value";
		gpio_export_name = "/sys/class/aml_gpio/export";
		gpio_direction_name = "/sys/class/aml_gpio/gpio%d/direction";
		gpio_edge_name = "/sys/class/aml_gpio/gpio%d/edge";
	}

	sprintf(path, gpio_value_name, pinToGpio[pin]);
	fd = open(path, O_RDWR);

	if(fd < 0) {
		if((f = fopen(gpio_export_name, "w")) == NULL) {
			wiringXLog(LOG_ERR, "odroid->isr: Unable to open GPIO export interface: %s", strerror(errno));
			return -1;
		}

		fprintf(f, "%d\n", pinToGpio[pin]);
		fclose(f);
	}

	sprintf(path, gpio_direction_name, pinToGpio[pin]);
	if((f = fopen(path, "w")) == NULL) {
		wiringXLog(LOG_ERR, "odroid->isr: Unable to open GPIO direction interface for pin %d: %s", pin, strerror(errno));
		return -1;
	}

	fprintf(f, "in\n");
	fclose(f);

	sprintf(path, gpio_edge_name, pinToGpio[pin]);
	if((f = fopen(path, "w")) == NULL) {
		wiringXLog(LOG_ERR, "odroid->isr: Unable to open GPIO edge interface for pin %d: %s", pin, strerror(errno));
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
		wiringXLog(LOG_ERR, "odroid->isr: Invalid mode: %s. Should be rising, falling or both", sMode);
		return -1;
	}
	fclose(f);

	if((f = fopen(path, "r")) == NULL) {
		wiringXLog(LOG_ERR, "odroid->isr: Unable to open GPIO edge interface for pin %d: %s", pin, strerror(errno));
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
		wiringXLog(LOG_ERR, "odroid->isr: Failed to set interrupt edge to %s", sMode);
		return -1;	
	}

	sprintf(path, gpio_value_name, pinToGpio[pin]);
	if((sysFds[pin] = open(path, O_RDONLY)) < 0) {
		wiringXLog(LOG_ERR, "odroid->isr: Unable to open GPIO value interface: %s", strerror(errno));
		return -1;
	}
	changeOwner(path);

	sprintf(path, gpio_edge_name, pinToGpio[pin]);
	changeOwner(path);

	ioctl(fd, FIONREAD, &count);
	for(i=0; i<count; ++i) {
		if (read(fd, &c, 1) == -1) {
			wiringXLog(LOG_ERR, "odroid->isr: read() failed");
			return -1;
		}
	}
	close(fd);

	return 0;
}

static int odroidWaitForInterrupt(int pin, int ms) {
	int x = 0;
	uint8_t c = 0;
	struct pollfd polls;

	if(odroidValidGPIO(pin) != 0) {
		wiringXLog(LOG_ERR, "odroid->waitForInterrupt: Invalid pin number %d", pin);
		return -1;
	}

	if(pinModes[pin] != SYS) {
		wiringXLog(LOG_ERR, "odroid->waitForInterrupt: Trying to read from pin %d, but it's not configured as interrupt", pin);
		return -1;
	}

	if(sysFds[pin] == -1) {
		wiringXLog(LOG_ERR, "odroid->waitForInterrupt: GPIO %d not set as interrupt", pin);
		return -1;
	}

	polls.fd = sysFds[pin];
	polls.events = POLLPRI;

	x = poll(&polls, 1, ms);

	/* Don't react to signals */
	if(x == -1 && errno == EINTR) {
		x = 0;
	}
	
	if (read(sysFds[pin], &c, 1) == -1) {
		wiringXLog(LOG_ERR, "odroid->waitForInterrupt: read() failed");
		return -1;
	}
	lseek(sysFds[pin], 0, SEEK_SET);

	return x;
}

static int odroidGC(void) {
	int i = 0, fd = 0;
	char path[35];
	FILE *f = NULL;

	for(i=0;i<NUM_PINS;i++) {
		if(pinModes[i] == OUTPUT) {
			pinMode(i, INPUT);
		} else if(pinModes[i] == SYS) {
			sprintf(path, "/sys/class/gpio/gpio%d/value", pinToGpio[i]);
			if((fd = open(path, O_RDWR)) > 0) {
				if((f = fopen("/sys/class/gpio/unexport", "w")) == NULL) {
					wiringXLog(LOG_ERR, "odroid->gc: Unable to open GPIO unexport interface: %s", strerror(errno));
				}

				fprintf(f, "%d\n", pinToGpio[i]);
				fclose(f);
				close(fd);
			}
		}
		if(sysFds[i] > 0) {
			close(sysFds[i]);
		}
	}

	if(gpio) {
		munmap((void *)gpio, BLOCK_SIZE);
	}
	return 0;
}

#ifndef __FreeBSD__
static int odroidI2CRead(int fd) {
	return i2c_smbus_read_byte(fd);
}

static int odroidI2CReadReg8(int fd, int reg) {
	return i2c_smbus_read_byte_data(fd, reg);
}

static int odroidI2CReadReg16(int fd, int reg) {
	return i2c_smbus_read_word_data(fd, reg);
}

static int odroidI2CWrite(int fd, int data) {
	return i2c_smbus_write_byte(fd, data);
}

static int odroidI2CWriteReg8(int fd, int reg, int data) {
	return i2c_smbus_write_byte_data(fd, reg, data);
}

static int odroidI2CWriteReg16(int fd, int reg, int data) {
	return i2c_smbus_write_word_data(fd, reg, data);
}

static int odroidI2CSetup(int devId) {
	int rev = 0, fd = 0;
	const char *device = NULL;
	static int boardRev = -1;

	if((rev = odBoardRev()) < 0) {
		wiringXLog(LOG_ERR, "odroid->I2CSetup: Unable to determine Pi board revision");
		return -1;
	}

	device = "/dev/i2c-1";

	if((fd = open(device, O_RDWR)) < 0) {
		wiringXLog(LOG_ERR, "odroid->I2CSetup: Unable to open %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, devId) < 0) {
		wiringXLog(LOG_ERR, "odroid->I2CSetup: Unable to set %s to slave mode: %s", device, strerror(errno));
		return -1;
	}

	return fd;
}

static int odroidSPIGetFd(int channel) {
	return spiFds[channel & 1];
}

static int odroidSPIDataRW(int channel, unsigned char *data, int len) {
	struct spi_ioc_transfer spi;
	memset(&spi, 0, sizeof(spi));
	channel &= 1;

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
		wiringXLog(LOG_ERR, "odroid->SPIDataRW: Unable to read/write from channel %d: %s", channel, strerror(errno));
		return -1;
	}
	return 0;
}

static int odroidSPISetup(int channel, int speed) {
	int fd;
	const char *device = NULL;

	channel &= 1;

	if (odModel == OD_MODEL_ODROIDXU_34) {
		device = "/dev/spidev1.0";
	} else if(channel == 0) {
		device = "/dev/spidev0.0";
	} else {
		device = "/dev/spidev0.1";
	}

	if((fd = open(device, O_RDWR)) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to open device %s: %s", device, strerror(errno));
		return -1;
	}

	spiSpeeds[channel] = speed;
	spiFds[channel] = fd;

	if(ioctl(fd, SPI_IOC_WR_MODE, &spiMode) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to set write mode for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_MODE, &spiMode) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to set read mode for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to set write bits_per_word for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spiBPW) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to set read bits_per_word for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to set write max_speed for device %s: %s", device, strerror(errno));
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
		wiringXLog(LOG_ERR, "odroid->SPISetup: Unable to set read max_speed for device %s: %s", device, strerror(errno));
		return -1;
	}

	return fd;

}
#endif

void odroidInit(void) {

	memset(pinModes, -1, sizeof(pinModes));

	platform_register(&odroid, "odroid");
	odroid->setup=&setup;
	odroid->pinMode=&odroidPinMode;
	odroid->analogRead=&odroidAnalogRead;
	odroid->digitalWrite=&odroidDigitalWrite;
	odroid->digitalRead=&odroidDigitalRead;
	odroid->identify=&odBoardRev;
	odroid->isr=&odroidISR;
	odroid->waitForInterrupt=&odroidWaitForInterrupt;
#ifndef __FreeBSD__
	odroid->I2CRead=&odroidI2CRead;
	odroid->I2CReadReg8=&odroidI2CReadReg8;
	odroid->I2CReadReg16=&odroidI2CReadReg16;
	odroid->I2CWrite=&odroidI2CWrite;
	odroid->I2CWriteReg8=&odroidI2CWriteReg8;
	odroid->I2CWriteReg16=&odroidI2CWriteReg16;
	odroid->I2CSetup=&odroidI2CSetup;
	odroid->SPIGetFd=&odroidSPIGetFd;
	odroid->SPIDataRW=&odroidSPIDataRW;
	odroid->SPISetup=&odroidSPISetup;
#endif
	odroid->gc=&odroidGC;
	odroid->validGPIO=&odroidValidGPIO;
}
