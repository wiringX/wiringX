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

# This example shows how to read temperature from the TMP102 I2C device
# with WiringX in Python. It assumes that the sensor is configured at 
# the default I2C address of 0x48

from time import sleep

from wiringX import gpio

# setup wiringX
gpio.setup()

# get a handle to the sensor, using the default I2C address
fd = gpio.I2CSetup(0x48)
while True:
    # read from the default register
    data = gpio.I2CReadReg16(fd, 0x00)
    reg = []
    # calculate the temperature
    reg.append((data>>8)&0xff)
    reg.append(data&0xff)
    res  = (reg[1] << 4) | (reg[0] >> 4)
    res = res * 0.0625

    # print the result
    print(u'Temperature: ' + str(res) + u' C')
    sleep(1)
