#!/usr/bin/env python
# Copyright (c) 2014 CurlyMo <curlymoo1@gmail.com>

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

import os
import sys
from time import sleep
from wiringX import gpio

gpio.setup();

gpio.pinMode(gpio.PIN0, gpio.OUTPUT);

print gpio.platform();
print gpio.I2CRead(0x10);
try:
	while True:
		gpio.digitalWrite(gpio.PIN0, gpio.LOW);
		sleep(1);
		gpio.digitalWrite(gpio.PIN0, gpio.HIGH);
		sleep(1);
except KeyboardInterrupt:
	pass
