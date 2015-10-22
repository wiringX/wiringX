#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "wiringX.h"

char *usage =
	"Usage: %s GPIO\n"
	"       GPIO is the GPIO to write to\n"
	"Example: %s 10\n";

int main(int argc, char *argv[]) {
	char *str = NULL;
	char usagestr[120];
	int gpio = 0, invalid = 0;

	memset(usagestr, '\0', 120);

	// expect only 1 argument => argc must be 2
	if(argc != 2) {
		snprintf(usagestr, 119, usage, argv[0], argv[0]);
		puts(usagestr);
		return -1;
	}

	// check for a valid, numeric argument
	str = argv[1];
	while(*str != '\0') {
		if(!isdigit(*str)) {
			invalid = 1;
		}
		str++;
	}
	if(invalid == 1) {
		printf("%s: Invalid GPIO %s\n", argv[0], argv[1]);
		return -1;
	}

	gpio = atoi(argv[1]);

	wiringXSetup();

	if(wiringXValidGPIO(gpio) != 0) {
		printf("%s: Invalid GPIO %d\n", argv[0], gpio);
		return -1;
	}

	pinMode(gpio, OUTPUT);
	while(1) {
		printf("Writing to GPIO %d: High\n", gpio);
		digitalWrite(gpio, HIGH);
		sleep(1);
		printf("Writing to GPIO %d: Low\n", gpio);
		digitalWrite(gpio, LOW);
		sleep(1);
	}
}
