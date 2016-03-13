/* Copyright (c) 2015 Paul Adams <paul@thoughtcriminal.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This example shows how to read from the TMP102 I2C temperature sensor 
 * and write to the Sparkfun 7-segment display
 * https://www.sparkfun.com/products/11442 using WiringX.
 *
 * Compile with gcc -Wall -o i2c_spi i2c_spi.c -lwiringX 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "wiringX.h"

/* I2C Config */
#define I2C_ADDR	0x48
#define TMP_WR	 	0x90 //Assume ADR0 is tied to VCC
#define TMP_RD	 	0x91
#define TEMP_REG 	0x00

/* SPI Config */
#define SPI_CHAN	0
#define SPI_SPEED	250000

int fd_i2c;
int fd_spi;

void clearDisplaySPI() {
	unsigned char spi_data[1] = {0x76};
	wiringXSPIDataRW(SPI_CHAN, spi_data, 1);  // Clear display command
}
void setCursorSPI() {
	unsigned char spi_data[2] = {0x79, 0x00};
	wiringXSPIDataRW(SPI_CHAN, spi_data, 2);  // set cursor command
}
void setDecimalsSPI(unsigned char decimals) {
	unsigned char spi_data[2] = {0x77, decimals};
	wiringXSPIDataRW(SPI_CHAN, spi_data, 2);
}

int main(void) {
	char reg[2] = {0,0};
	int i2c_data = 0;
	unsigned char spi_data[4] = "0000";
	char temp_str[5];

	if(wiringXSetup("pcduino1", NULL) == -1) {
		wiringXGC();
		return -1;
	}

	// set up I2C
	if((fd_i2c = wiringXI2CSetup("/dev/i2c-0", I2C_ADDR)) < 0) {
		fprintf(stderr, "I2C Setup failed: %i\n", fd_i2c);
		return -1;
	} else {
		fprintf(stderr, "I2C Setup OK: %i\n", fd_i2c);
	}

	// set up SPI
	if((fd_spi = wiringXSPISetup(SPI_CHAN, SPI_SPEED)) < 0) {
		fprintf(stderr, "SPI Setup failed: %i\n", fd_spi);
	} else {
		fprintf(stderr, "SPI Setup OK: %i\n", fd_spi);
	}

	// clear the display
	clearDisplaySPI();

	while(1) {
		// read the data from TMP102
		i2c_data = wiringXI2CReadReg16(fd_i2c, TEMP_REG);

		// calculate the temperature
		reg[0] = (i2c_data>>8)&0xff;
		reg[1] = i2c_data&0xff;
		int16_t res  = ((int8_t)reg[1] << 4) | ((uint8_t)reg[0] >> 4);
		float temp =  (float)res * 0.0625;
		fprintf(stderr, "Temperature = %2.2f\n", temp);

		// create string to send to the 7-segment display
		sprintf(temp_str, "%2.2f", temp);
		spi_data[0] = (temp_str[0] - '0') & 0xff;
		spi_data[1] = (temp_str[1] - '0') & 0xff;
		spi_data[2] = (temp_str[3] - '0') & 0xff;
		spi_data[3] = (temp_str[4] - '0') & 0xff;

		// set cursor to beginning
		setCursorSPI();

		// send data to 7-segment display
		wiringXSPIDataRW(SPI_CHAN, spi_data, 4);

		// set the decimal point
		setDecimalsSPI(2&0xff);
		sleep(2);
	}
}

