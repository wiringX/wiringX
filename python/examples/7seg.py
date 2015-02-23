#!/usr/bin/env python
# Copyright (c) 2015 Paul Adams <paul@thoughtcriminal.co.uk>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# This example shows how to write to the Sparkfun 7-segment display
# https://www.sparkfun.com/products/11442 with WiringX in Python.

from time import sleep

from wiringX import gpio

# setup wiringX
gpio.setup()

# set up the SPI device
fd = gpio.SPISetup(0, 250000)

while True:
    # write 1 2 3 4 to the display
    data = gpio.SPIDataRW(0, bytearray([0x01,0x02,0x03,0x04]), 4)

    # set the decimal point to position 2
    data = gpio.SPIDataRW(0, bytearray([0x77,0x02]), 2)
    sleep(1)

    # clear the display
    data = gpio.SPIDataRW(0, bytearray([0x76]), 1)
    sleep(1)
