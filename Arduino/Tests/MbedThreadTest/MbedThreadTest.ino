/*
  3DLiveIMUTest

  - Testing thread functions of mbed os

  The circuit:
  * Board: Arduino Nano 33 BLE Sense

  Created 12 02 2021
  By Johannes Brunner

*/

#include <mbed.h>
#include "MemoryFree.h"


using namespace rtos;
using namespace mbed;

static Thread* t = NULL;

bool x = false;



void setup() {
	while (!Serial);
}
void loop() {
	while (true) {
		Serial.println("Main running");
		if (!x) {
			Serial.println("create thread");
			delete t;
			t = new Thread();
			t->start(threadFunction);
			x = true;
		}
		thread_sleep_for(500);
	}
}

void threadFunction() {
	Serial.println(freeMemory());
	thread_sleep_for(100);
	Serial.println("T end");
	x = false;
	t->terminate();
};
