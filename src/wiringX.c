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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "wiringX.h"
#include "hummingboard.h"
#include "raspberrypi.h"
#include "bananapi.h"
#include "radxa.h"
#include "ci20.h"

#ifdef _WIN32
#define timeradd(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
        if ((result)->tv_usec >= 1000000L) { \
            ++(result)->tv_sec; \
            (result)->tv_usec -= 1000000L; \
        } \
    } while (0)

#define timersub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_usec += 1000000L; \
        } \
    } while (0)
#endif

static struct platform_t *platform = NULL;
#ifndef _WIN32
	static int setup = -2;
#endif

void _fprintf(int prio, const char *format_str, ...) {
	char line[1024];
	va_list ap;
	va_start(ap, format_str);
	vsprintf(line, format_str, ap);
	strcat(line, "\n");
	fprintf(stderr, line);
	va_end(ap);
}

/* Both the delayMicroseconds and the delayMicrosecondsHard
   are taken from wiringPi */
static void delayMicrosecondsHard(unsigned int howLong) {
	struct timeval tNow, tLong, tEnd ;

	gettimeofday(&tNow, NULL);
#ifdef _WIN32
	tLong.tv_sec  = howLong / 1000000;
	tLong.tv_usec = howLong % 1000000;
#else
	tLong.tv_sec  = (__time_t)howLong / 1000000;
	tLong.tv_usec = (__suseconds_t)howLong % 1000000;
#endif
	timeradd(&tNow, &tLong, &tEnd);

	while(timercmp(&tNow, &tEnd, <)) {
		gettimeofday(&tNow, NULL);
	}
}

void delayMicroseconds(unsigned int howLong) {
	struct timespec sleeper;
#ifdef _WIN32
	long int uSecs = howLong % 1000000;
	unsigned int wSecs = howLong / 1000000;
#else
	long int uSecs = (__time_t)howLong % 1000000;
	unsigned int wSecs = howLong / 1000000;
#endif

	if(howLong == 0) {
		return;
	} else if(howLong  < 100) {
		delayMicrosecondsHard(howLong);
	} else {
#ifdef _WIN32
		sleeper.tv_sec = wSecs;
#else
		sleeper.tv_sec = (__time_t)wSecs;	
#endif
		sleeper.tv_nsec = (long)(uSecs * 1000L);
		nanosleep(&sleeper, NULL);
	}
}

void platform_register(struct platform_t **dev, const char *name) {
	*dev = malloc(sizeof(struct platform_t));
	(*dev)->name = NULL;
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
	(*dev)->SPIGetFd = NULL;
	(*dev)->SPIDataRW = NULL;
	(*dev)->analogRead = NULL;
	(*dev)->pwmEnable = NULL;
	(*dev)->setPWM = NULL;

	if(((*dev)->name = malloc(strlen(name)+1)) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(EXIT_FAILURE);
	}
	strcpy((*dev)->name, name);
	(*dev)->next = platforms;
	platforms = (*dev);
}

int wiringXGC(void) {
	int i = 0;
	if(platform != NULL) {
		i = platform->gc();
	}
	platform = NULL;
	struct platform_t *tmp = platforms;
	while(platforms) {
		tmp = platforms;
		free(platforms->name);
		platforms = platforms->next;
		free(tmp);
	}
	if(platforms != NULL) {
		free(platforms);
	}

	wiringXLog(LOG_DEBUG, "garbage collected wiringX library");
	return i;
}

void pinMode(int pin, int mode) {
	if(platform != NULL) {
		if(platform->pinMode) {
			if(platform->pinMode(pin, mode) == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling pinMode", platform->name);
				wiringXGC();
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support pinMode", platform->name);
			wiringXGC();
		}
	}
}

int wiringXAnalogRead(int channel){
	if(platform != NULL) {
		if(platform->analogRead) {
			int x = platform->analogRead(channel);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling analogRead", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support analogRead", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

void digitalWrite(int pin, int value) {
	if(platform != NULL) {
		if(platform->digitalWrite) {
			if(platform->digitalWrite(pin, value) == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling digitalWrite", platform->name);
				wiringXGC();
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support digitalWrite", platform->name);
			wiringXGC();
		}
	}
}

int digitalRead(int pin) {
	if(platform != NULL) {
		if(platform->digitalRead) {
			int x = platform->digitalRead(pin);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling digitalRead", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support digitalRead", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int waitForInterrupt(int pin, int ms) {
	if(platform != NULL) {
		if(platform->waitForInterrupt) {
			int x = platform->waitForInterrupt(pin, ms);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling waitForInterrupt", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support waitForInterrupt", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXISR(int pin, int mode) {
	if(platform != NULL) {
		if(platform->isr) {
			int x = platform->isr(pin, mode);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling isr", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support isr", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CRead(int fd) {
	if(platform != NULL) {
		if(platform->I2CRead) {
			int x = platform->I2CRead(fd);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CRead", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CRead", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CReadReg8(int fd, int reg) {
	if(platform != NULL) {
		if(platform->I2CReadReg8) {
			int x = platform->I2CReadReg8(fd, reg);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CReadReg8", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CReadReg8", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CReadReg16(int fd, int reg) {
	if(platform != NULL) {
		if(platform->I2CReadReg16) {
			int x = platform->I2CReadReg16(fd, reg);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CReadReg16", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CReadReg16", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CWrite(int fd, int data) {
	if(platform != NULL) {
		if(platform->I2CWrite) {
			int x = platform->I2CWrite(fd, data);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CWrite", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CWrite", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CWriteReg8(int fd, int reg, int data) {
	if(platform != NULL) {
		if(platform->I2CWriteReg8) {
			int x = platform->I2CWriteReg8(fd, reg, data);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CWriteReg8", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CWriteReg8", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CWriteReg16(int fd, int reg, int data) {
	if(platform != NULL) {
		if(platform->I2CWriteReg16) {
			int x = platform->I2CWriteReg16(fd, reg, data);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CWriteReg16", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CWriteReg16", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXI2CSetup(int devId) {
	if(platform != NULL) {
		if(platform->I2CSetup) {
			int x = platform->I2CSetup(devId);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling I2CSetup", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support I2CSetup", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXSPIGetFd(int channel) {
	if(platform != NULL) {
		if(platform->SPIGetFd) {
			int x = platform->SPIGetFd(channel);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling SPIGetFd", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support SPIGetFd", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXSPIDataRW(int channel, unsigned char *data, int len) {
	if(platform != NULL) {
		if(platform->SPIDataRW) {
			int x = platform->SPIDataRW(channel, data, len);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling SPIDataRW", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support SPIDataRW", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXSPISetup(int channel, int speed) {
	if(platform != NULL) {
		if(platform->SPISetup) {
			int x = platform->SPISetup(channel, speed);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling SPISetup", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support SPISetup", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXSerialOpen(char *device, struct wiringXSerial_t wiringXSerial) {
	struct termios options;
	speed_t myBaud;
	int status = 0, fd = 0;

	switch(wiringXSerial.baud) {
		case     50:	myBaud = B50; break;
		case     75:	myBaud = B75; break;
		case    110:	myBaud = B110; break;
		case    134:	myBaud = B134; break;
		case    150:	myBaud = B150; break;
		case    200:	myBaud = B200; break;
		case    300:	myBaud = B300; break;
		case    600:	myBaud = B600; break;
		case   1200:	myBaud = B1200; break;
		case   1800:	myBaud = B1800; break;
		case   2400:	myBaud = B2400; break;
		case   4800:	myBaud = B4800; break;
		case   9600:	myBaud = B9600; break;
		case  19200:	myBaud = B19200; break;
		case  38400:	myBaud = B38400; break;
		case  57600:	myBaud = B57600; break;
		case 115200:	myBaud = B115200; break;
		case 230400:	myBaud = B230400; break;

		default:
		return -2;
	}

	if((fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1) {
		return -1;
	}

	fcntl(fd, F_SETFL, O_RDWR);

	tcgetattr(fd, &options);

	cfmakeraw(&options);
	cfsetispeed(&options, myBaud);
	cfsetospeed(&options, myBaud);

	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~CSIZE;
	switch(wiringXSerial.databits) {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			wiringXLog(LOG_ERR, "Unsupported data size");
			return -1;
	}
	switch(wiringXSerial.parity) {
		case 'n':
		case 'N':/*no parity*/
			options.c_cflag &= ~PARENB;
			options.c_iflag &= ~INPCK;
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);
			options.c_iflag |= INPCK; /* Disable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB; /* Enable parity */
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;
			break;
		case 'S':
		case 's': /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			wiringXLog(LOG_ERR, "Unsupported parity");
			return -1;
	}
	switch(wiringXSerial.stopbits) {
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			wiringXLog(LOG_ERR, "Unsupported data stop bits");
			return -1;
	}
	switch(wiringXSerial.flowcontrol) {
		case 'x':
		case 'X':
			options.c_iflag |= (IXON | IXOFF | IXANY);
			break;
		case 'n':
		case 'N':
			options.c_iflag &= ~(IXON | IXOFF | IXANY);
			break;
		default:
			wiringXLog(LOG_ERR, "Unsupported data flow control");
			return -1;
	}

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 150;

	tcflush(fd,TCIFLUSH);

	tcsetattr(fd, TCSANOW | TCSAFLUSH, &options);

	ioctl(fd, TIOCMGET, &status);

	status |= TIOCM_DTR;
	status |= TIOCM_RTS;

	ioctl(fd, TIOCMSET, &status);

	return fd;
}

void wiringXSerialFlush(int fd) {
	if(fd > 0) {
		tcflush(fd, TCIOFLUSH);
	} else {
		wiringXLog(LOG_ERR, "Serial interface is not opened");
	}
}

void wiringXSerialClose(int fd) {
	if(fd > 0) {
		close(fd);
	}
}

void wiringXSerialPutChar(int fd, unsigned char c) {
	if(fd > 0) {
		write(fd, &c, 1);
	} else {
		wiringXLog(LOG_ERR, "Serial interface is not opened");
	}
}

void wiringXSerialPuts(int fd, char *s) {
	if(fd > 0) {
		write(fd, s, strlen(s));
	} else {
		wiringXLog(LOG_ERR, "Serial interface is not opened");
	}
}

void wiringXSerialPrintf(int fd, char *message, ...) {
	va_list argp;
	char buffer[1024];

	memset(&buffer, '\0', 1024);

	if(fd > 0) {
		va_start(argp, message);
		vsnprintf(buffer, 1023, message, argp);
		va_end(argp);

		wiringXSerialPuts(fd, buffer);
	} else {
		wiringXLog(LOG_ERR, "Serial interface is not opened");
	}
}

int wiringXSerialDataAvail(int fd) {
	int result = 0;

	if(fd > 0) {
		if(ioctl(fd, FIONREAD, &result) == -1) {
			return -1;
		}
		return result;
	} else {
		wiringXLog(LOG_ERR, "Serial interface is not opened");
		return -1;
	}
}

int wiringXSerialGetChar(int fd) {
	uint8_t x = 0;

	if(fd > 0) {
		if(read(fd, &x, 1) != 1) {
			return -1;
		}
		return ((int)x) & 0xFF;
	} else {
		wiringXLog(LOG_ERR, "Serial interface is not opened");
		return -1;
	}
}

//PWM
int wiringXPWMEnable(int pin, int enable){
	if(platform != NULL) {
		if(platform->pwmEnable) {
			int x = platform->pwmEnable(pin, enable);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling pwmEnable", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support pwmEnable", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int  wiringXSetPWM(int pin, float period_ms, float duty){
	if(platform != NULL) {
		if(platform->setPWM) {
			int x = platform->setPWM(pin, period_ms, duty);
			if(x == -1) {
				wiringXLog(LOG_ERR, "%s: error while calling setPWM", platform->name);
				wiringXGC();
			} else {
				return x;
			}
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support setPWM", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

char *wiringXPlatform(void) {
	if(platform != NULL) {
		return platform->name;
	} else {
		return NULL;
	}
}

int wiringXValidGPIO(int gpio) {
	if(platform != NULL) {
		if(platform->validGPIO) {
			return platform->validGPIO(gpio);
		} else {
			wiringXLog(LOG_ERR, "%s: platform doesn't support gpio number validation", platform->name);
			wiringXGC();
		}
	}
	return -1;
}

int wiringXSupported(void) {
#if defined(__arm__) || defined(__mips__)
	return 0;
#else
	return -1;
#endif
}

int wiringXSetup(void) {
#ifndef _WIN32	
	if(wiringXLog == NULL) {
		wiringXLog = _fprintf;
	}

	if(wiringXSupported() == 0) {
		if(setup == -2) {
			hummingboardInit();
			raspberrypiInit();
			bananapiInit();
			ci20Init();
			radxaInit();

			int match = 0;
			struct platform_t *tmp = platforms;
			while(tmp) {
				if(tmp->identify() >= 0) {
					platform = tmp;
					match = 1;
					break;
				}
				tmp = tmp->next;
			}

			if(match == 0) {
				wiringXLog(LOG_ERR, "hardware not supported");
				wiringXGC();
				return -1;
			} else {
				wiringXLog(LOG_DEBUG, "running on a %s", platform->name);
			}
			setup = platform->setup();
			return setup;
		} else {
			return setup;
		}
	}
#endif
	return -1;
}
