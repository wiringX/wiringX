Rules for contributing to wiringX
=================================
First of all, thanks for making this project better. Without your help, wiringX and all project using it, wouldn't have come to what it is now!

**BUT**, be aware that contributing to a big project shouldn't be considered a playground. It also isn't meant for you to learn `git`.
So if you don't know what you are doing than don't do it. Instead open an [forum topic](http://forum.pilight.org), point to your code, and ask for help.

New Modules
---------

1. We only accept memory mapped GPIO reading and writing, interrupts can use sysfs.
2. New platforms must be added two folded. A new SoC module and a platform module.
If the SoC is already supported, you can use the existing implementation.
3. The SoC used must be fully mapped in a `separate` module, and should be mapped
as simple as possible. So no formulas, but direct memory areas. Look at the
existing modules for examples. Also don't 'hack' things into the SoC module,
because your platform needs it. Instead move it to the platform module.
4. The new platform should map the SoC GPIO to the platform specific GPIO.
5. The new platform module must use as much of the existing library as
possible. Don't implement functionality that already exists, except when
absolutely necessary.
6. If your platform has (nearly) the same layout as already supported platforms,
then make sure you use the same numbering.
7. The platform module must include at least support for:
- pinMode
- digitalWrite
- digitalRead
- waitForInterrupt
- wiringXISR
- wiringXValidGPIO
- wiringXGC
8. Your module must pass the read, blink, and interrupt example tests.
9. Add your new module to wiringX.c and wiringX.h.
10. Add your new module to the sources list in python/setup.py
11. You agree to publish your code under the MPLv2 license, and
have added the MPLv2 header accordingly as well as a copyright
notice.

The output of the `interrupt` example should look like this and nothing else
(the GPIO numbers of course depend on the GPIO used):
```
Thread created succesfully
  Writing to GPIO 1: High
  Writing to GPIO 1: Low
>>Interrupt on GPIO 0
  Timeout on GPIO 0
  Writing to GPIO 1: High
>>Interrupt on GPIO 0
  Writing to GPIO 1: Low
>>Interrupt on GPIO 0
  Timeout on GPIO 0
  Writing to GPIO 1: High
>>Interrupt on GPIO 0
  Writing to GPIO 1: Low
>>Interrupt on GPIO 0
  Timeout on GPIO 0
  Writing to GPIO 1: High
>>Interrupt on GPIO 0
  Writing to GPIO 1: Low
>>Interrupt on GPIO 0
  Timeout on GPIO 0
  Writing to GPIO 1: High
>>Interrupt on GPIO 0
  Writing to GPIO 1: Low
>>Interrupt on GPIO 0
  Timeout on GPIO 0
Main finished, waiting for thread ...
  Timeout on GPIO 0
  Timeout on GPIO 0
  Timeout on GPIO 0
  Timeout on GPIO 0
  Timeout on GPIO 0
```

Documentation
--------
The wiringX documentation can be found here:
https://manual.wiringx.org
https://github.com/wiringX/wiringx-manual

1. Create an OpenDocument fodg vector for your specific platform.
Re-use vectors that already exists for the supported platforms.
2. Create an OpenDocument fodt page in the same manner as the
existing platforms. Include the platform vector as an SVG image
and not as a binary.
3. Don't embed the fonts in the OpenDocument fodt file.
4. Concisely describe the function of each GPIO as found in the
other platforms.
5. Create an OpenDocument fodt SoC page that describes how a specific
SoC was mapped into a wiringX module. Again look at existing
examples and copy that structure.

Extending existing modules
--------
If you extend existing modules make sure to:

1. Make sure to update the documentation.
2. Add new methods to README.md
3. Add Python bindings in python/wiringX/wiringx.c and test for compatibiity with Python2 and Python3

Pull Request Checklist
---------
When you are ready to do your pull-request, check the following list:

1. Keep the coding style in sync with that of wiringX (see below).
2. First merge with the latest wiringX code.
3. Make a difference branch for each new feature you want to commit.
5. Test how pull-requests work on your own test repositories.
6. Make sure your pull-request contains [one single commit](http://eli.thegreenplace.net/2014/02/19/squashing-github-pull-requests-into-a-single-commit).
7. Open a pull-request when you indeed want to contribute and follow-up on our comments. If you don't want to implemented our requested changes after reviewing your pull-request, don't bother opening one.
8. Re-read this file before every pull-request, because it will be updated regularly.
9. Don't forget to enjoy the appreciation of the end user!
10. **If you don't follow-up on these rules, your PR will be closed and ignored**

Coding style
-----
- No unnecessary spaces
```
if ( 1 == 1 )
{
...
}
```
should become:
```
if(1 == 1) {
...
}
```
- Don't inline variables:
```
int x                  = 0;
int long_variable_name = 0;
int a                  = 0;
```
but use
```
int x =	0;
int long_variable_name = 0;
int a = 0;
```
but preferable use for around max 50 characters:
```
int x = 0, long_variable_name = 0, a = 0;
```
- Variable defining order.
First start with `struct`, then specials types (`*_t`), then `char`, then `double` / `float` and end with `int`.
```
	struct protocol_threads_t *node = (struct protocol_threads_t *)param;
	struct JsonNode *json = (struct JsonNode *)node->param;
	struct JsonNode *jid = NULL;
	struct JsonNode *jchild = NULL;
	time_t time;
	char *tmp = NULL;
	double itmp = 0.0;
	int id = 0, state = 0, nstate = 0;
```
- Initialize your variables.
```
char *a = NULL;
char a[10];
memset(&a, 0, 10);
double a = 0.0;
int a = 0;
```
- User the `static` keyword for all variables and functions appropriatly.
- Always use tabs instead of spaces for inline markup.
