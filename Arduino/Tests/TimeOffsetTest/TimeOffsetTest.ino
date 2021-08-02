#include "mbed.h"
#include "ArduinoBLE.h"
#include "PDMdup.h"

void setup() {
	while (!Serial);
	Serial.begin(115200);
	set_time(1256729737);  // Set RTC time to Wed, 28 Oct 2009 11:35:37

	long startMillis = millis();

	Serial.println(NRF_CLOCK->LFCLKSTAT);
	Serial.println(NRF_CLOCK->LFCLKSRCCOPY);

	/*
	BLE.begin();
	while (millis() < (startMillis + 5000)) {
		BLE.poll();
	}
	BLE.end();
	Serial.println("BLE over");
	*/

	
	PDM.begin(1, 16000);
	while (millis() < (startMillis + 5000)) {
	}
	PDM.end();
	Serial.println("PDM over");
	
	Serial.println(NRF_CLOCK->LFCLKSTAT);
	Serial.println(NRF_CLOCK->LFCLKSRCCOPY);

	NRF_CLOCK->TASKS_LFCLKSTOP = 1;
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;

	NRF_CLOCK->LFCLKSRC = 1;

	NRF_CLOCK->TASKS_LFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {};

	NRF_CLOCK->TASKS_HFCLKSTOP = 1;
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
};

void loop() {
	time_t seconds = time(NULL);

	//Serial.printf("Time as seconds since January 1, 1970 = %u\n", (unsigned int)seconds);

   // Serial.printf("Time as a basic string = %s", ctime(&seconds));

	char buffer[32];
	strftime(buffer, 32, "%T\n", localtime(&seconds));
	Serial.printf("Time as a custom formatted string = %s", buffer);

	Serial.println(NRF_CLOCK->LFCLKSTAT);
	Serial.println(NRF_CLOCK->LFCLKSRCCOPY);

	thread_sleep_for(10000);

};
