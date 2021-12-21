/*
  TempTest

  - Testing Temperature and Humidity (HTS221)

  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Use internal sensor (Wire1)
  * Use external sensor (Wire)

  Created 02 08 2021
  By Johannes Brunner

*/


#include "Wire.h"
#include "HTS221.h"
#include "HTS.h"


void setup() {
	while (!Serial) {};
	Serial.begin(115200);
	digitalWrite(5, HIGH);
	delay(100);
}

void loop() {
	Wire1.begin();
	smeHumidity.setWire(&Wire1);

	if (!smeHumidity.begin()) {
		Serial.println("Failed to initialize humidity temperature sensor!");
		while (1);
	}
	Serial.print(smeHumidity.readTemperature());
	Serial.print(" | ");
	Serial.print(smeHumidity.readHumidity());
	Serial.print("       |        ");

	uint32_t startMillis = millis();
	while ((millis() - startMillis) < 1000) {};


	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 500) {};

	Serial.print(smeHumidity.readTemperature());
	Serial.print(" | ");
	Serial.println(smeHumidity.readHumidity());
	Wire1.end();

	delay(1000);
}
