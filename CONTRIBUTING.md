Rules for contributing to wiringX
=================================
First of all, thanks for making this project better. Without your help, wiringX and all project using it, wouldn't have come to what it is now!

**BUT**, be aware that contributing to a big project shouldn't be considered a playground. It also isn't meant for you to learn `git`.
So if you don't know what you are doing than don't do it. Instead open an [forum topic](http://forum.pilight.org), point to your code, and ask for help.

New Modules
---------

1. We only accept memory mapped GPIO reading and writing.
2. The module must include at least support for:
- wiringXSetup
- pinMode
- digitalWrite
- digitalRead
- delayMicroseconds
- waitForInterrupt
- wiringXISR
- wiringXValidGPIO
- wiringXGC
- wiringXPlatform
3. Your module must pass the read, blink, and interrupt example tests.
4. Your module must be able to discover it's own platform in whatever way. Most modules use /proc/cpuinfo for this.
5. Add your new module to wiringX.c and wiringX.h.
6. Add your new module to the sources list in python/setup.py
7. Add your new module to README.md
8. Add the GPIO mapping of your new module in index.html on the gh-pages branch.

Extending existing modules
--------
If you extend existing modules make sure to:

1. Add the GPIO mapping of your new module in index.html on the gh-pages branch.
2. Document new methods in index.html on the gh-pages branch
3. Add new methods to README.md
4. Add Python bindings in python/wiringX/wiringx.c and test for compatibiity with Python2 and Python3

Pull Request Checklist
---------
When you are ready to do your pull-request, check the following list:

1. Keep the coding style in sync with that of pilight (see below).
2. First merge with the latest wiringX code.
3. Make a difference branch for each new feature you want to commit.
5. Test how pull-requests work on your own test repositories.
6. Make sure your pull-request contains [one single commit](http://eli.thegreenplace.net/2014/02/19/squashing-github-pull-requests-into-a-single-commit).
7. Open a pull-request when you indeed want to contribute and follow-up on our comments. If you don't want to implemented our requested changes after reviewing your pull-request, don't bother opening one.
8. Re-read this file before every pull-request, because it will be updated regularly.
9. Don't forget to enjoy the appreciation of the end user!

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
- User the `static` keyword for all variables and function only use in the single C file your module consists of.
- Always use tabs instead of spaces for inline markup.
