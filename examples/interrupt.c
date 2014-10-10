#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "wiringX.h"

void *interrupt(void *param) {
	while(1) {
		if(waitForInterrupt(1, 1000) > 0) {
			printf("interrupt\n");
		} else {
			printf("timeout\n");
		}
	}
}

int main(void) {
	pthread_t pth;

	wiringXSetup();

	pinMode(0, OUTPUT);
	wiringXISR(1, INT_EDGE_BOTH);

	pthread_create(&pth, NULL, interrupt, NULL);

	while(1) {
		digitalWrite(0, HIGH);
		sleep(1);
		digitalWrite(0, LOW);
		sleep(2);
	}
}
