:tocdepth: 5

Welcome to wiringX documentation!
===================================

.. raw:: latex

   \newpage

Overview
--------

wiringX is a library that allows developers to control the GPIO of various platforms with generic and uniform functions. By using wiringX, the same code will run on all platforms supported by wiringX, natively. The wiringX core consists of three parts.

#. The GPIO of a SoC is fully mapped in a wiringX SoC module.
#. A platform module is created that uses the SoC module. A mask is put on the SoC GPIO representing the platform specific external GPIOs.
#. All platform specific functions are abstract in uniform wiringX functions.

wiringX is licensed under MPLv2 so both commercial and non-commercial parties are free to use the library in whatever way possible. The code can be found on Github as well as the source of this manual. The code of wiringX is intentionally kept as simple as possible. This is especially visible in the SoC mapping. Different SoCs use various formulas to target specific GPIO memory areas. wiringX does not use these formulas,  but registers all memory areas explicitly. This makes is easy to understand what is happening under the hood.

The power of wiringX consists in the support of as many platforms as possible. Various companies have sponsored the wiringX project with free products: SolidRun, LeMaker, and Radxa. The other devices where  bought by the wiringX project. Everyone is free to add new platforms and/or SoCs. If you want to let the wiringX project do it for you, feel free to sponsor us with specific devices.

Supported functions
-------------------

**General**

- wiringXGC
- wiringXPlatform
- wiringXSelectableFd
- wiringXSetup
- wiringXValidGPIO
- delayMicroseconds

**GPIO**

- pinMode
- digitalWrite
- digitalRead
- waitForInterrupt
- wiringXISR

**I2c**

- wiringXI2CRead
- wiringXI2CReadReg8
- wiringXI2CReadReg16
- wiringXI2CWrite
- wiringXI2CWriteReg8
- wiringXI2CWriteReg16
- wiringXI2CSetup

**SPI**

- wiringXSPIGetFd
- wiringXSPIDataRW
- wiringXSPISetup

**Serial**

- wiringXSerialOpen
- wiringXSerialFlush
- wiringXSerialClose
- wiringXSerialPutChar
- wiringXSerialPuts
- wiringXSerialPrintf
- wiringXSerialDataAvail
- wiringXSerialGetChar

Sitemap
-------

.. toctree::
   :maxdepth: 5
   :titlesonly:

   platforms/bananapi/index
   platforms/hummingboard/index
   platforms/odroid/index
   platforms/orangepi/index
   platforms/pcduino/index
   platforms/raspberrypi/index

