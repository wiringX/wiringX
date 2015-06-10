wiringX
========

wiringX is a modular approach to several GPIO interfaces.
wiringX will export all common GPIO functions also found libraries such as wiringPi
and wiringHB, but when using wiringX it will automatically determine what device 
your program is running on and use the appropriate GPIO functions. So when using 
wiringX, your program will just work in regard of GPIO functionality.

The wiringPi and wiringHB are almost a direct copy of their initial library.
However, wiringX currently does not yet support all features of the
Hummingboard and Raspberry Pi I/O. Therefore, wiringPi has been
stripped so it only supports those features also supported by wiringX.

Those features currently are:
- GPIO reading, writing, and interrupts.
- IC2 reading and writing.

The supported devices are:
- Raspberry Pi
- Hummingboard
- BananaPi
- Radxa Rock 
- MIPS CI20 Creator

###Donations

donate@pilight.org

###Installation:

* Let it automatically generate a deb package:
```
sudo ./gen.package.sh
sudo dpkg -i libwiringx*.deb
```
* Or by just compiling and running make install:
```
sudo cmake .
sudo make install
```
Make sure you have cmake in both cases installed:
```
sudo apt-get install cmake
```

wiringX is available in the Arch Linux ARM repository. To install, simply:
```
pacman -S wiringx-git
```
Pin numbering of the Raspberry Pi, Hummingboard and BananaPi can be found here:
https://projects.drogon.net/raspberry-pi/wiringpi/pins/

Pin numbering for the Radxa Rock can be found here:
http://radxa.com/Rock/extension_header
