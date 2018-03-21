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
from wiringX.gpio import gpio

def interrupt(x):
	if x > 0:
		print "interrupt"
	else:
		print "timeout"

gpio.setup(gpio.RASPBERRYPI1B2);

gpio.pinMode(gpio.PIN0, gpio.PINMODE_OUTPUT);
gpio.wiringXISR(gpio.PIN1, gpio.ISR_EDGE_BOTH);

try:
	gpio.waitForInterrupt(interrupt, gpio.PIN1, 1000);
	while True:
		gpio.digitalWrite(gpio.PIN0, gpio.HIGH);
		sleep(1);
		gpio.digitalWrite(gpio.PIN0, gpio.LOW);
		sleep(2);	
except KeyboardInterrupt:
	pass;
