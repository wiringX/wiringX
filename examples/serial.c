#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "wiringX.h"

int fd=-1, c=0;

void *interrupt(void *param) {
	while(1){
		if(wiringXserialDataAvail (fd)>0){
			c=wiringXserialGetchar(fd);
			printf("Data received is: %d.\n", c);
			sleep(1);
		}
	}
}

int main(void) {
	pthread_t pth;
	unsigned char d=0x00;

	wiringXSetup();
	pinMode(0, OUTPUT);
	if ((fd = wiringXserialOpen ("/dev/ttyS0", 9600)) < 0)	{
	    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
	    return -1;
	}

	pthread_create(&pth, NULL, interrupt, NULL);

	wiringXserialPutchar(fd, d);
	while(1){
		digitalWrite(0, HIGH);
		sleep(1);
		digitalWrite(0, LOW);
		sleep(1);
	}
}
