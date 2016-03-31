#!/usr/bin/env python
#
#	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>
#
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import sys
from time import sleep
from wiringX import gpio

gpio.setup(gpio.RASPBERRYPI1B2);

gpio.pinMode(gpio.PIN0, gpio.PINMODE_OUTPUT);

try:
	while True:
		gpio.digitalWrite(gpio.PIN0, gpio.HIGH);
		sleep(1);
		gpio.digitalWrite(gpio.PIN0, gpio.LOW);
		sleep(1);
except KeyboardInterrupt:
	pass
