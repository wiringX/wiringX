/*
	Copyright (c) 2015 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _WIRING_X_I2C_DEV_H_
#define _WIRING_X_I2C_DEV_H_

#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

extern inline __s32 i2c_smbus_access(int fd, char rw, int cmd, int size, union i2c_smbus_data *data);
extern inline __s32 i2c_smbus_read_byte(int fd);
extern inline __s32 i2c_smbus_write_byte(int fd, int value);
extern inline __s32 i2c_smbus_read_byte_data(int fd, int cmd);
extern inline __s32 i2c_smbus_write_byte_data(int fd, int cmd, int value);
extern inline __s32 i2c_smbus_read_word_data(int fd, int cmd);
extern inline __s32 i2c_smbus_write_word_data(int fd, int cmd, __u16 value);

#endif