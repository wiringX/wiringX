This package export all wiringX function to python.

Run "python setup.py install" to install.

On Arch Linux ARM, install "python-wiringx-git" or "python2-wiringx-git" via Pacman

To import:
   from wiringX import gpio

Function list:
- gpio.setup()
- gpio.pinMode(pin, mode)
- gpio.digitalWrite(pin, state)
- gpio.digitalRead(pin)
- gpio.valid(pin)
- gpio.gc()
- gpio.platform()
- gpio.I2CSetup(address)
- gpio.I2CRead(address)
- gpio.I2CReadReg8(fd, address)
- gpio.I2CReadReg16(fd, address)
- gpio.I2CWrite(fd, data)
- gpio.I2CWriteReg8(fd, address, data)
- gpio.I2CWriteReg16(fd, address, data)
- gpio.SPISetup(channel, speed_in_hz)
- gpio.SPIDataRW(channel, data, length)
- gpio.SPIGetFd(channel)
