/*
  BLECentralTest

  - A test for using the arduino as central device (client) in ble
  
  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Second ble-device acting as peripheral

  Created 18 11 2021
  By Johannes Brunner

*/

#include <ArduinoBLE.h>

void setup() {
	Serial.begin(9600);
	while (!Serial);

	// begin initialization
	if (!BLE.begin()) {
		Serial.println("starting BLE failed!");

		while (1);
	}

	Serial.println("BLE Central scan");

	// start scanning for peripheral
	//NRF_RADIO->TXPOWER = 8;
	//BLE.scan();

	BLE.scanForUuid("f1111a7f-0000-41f9-a127-aedf35a799b3");
}

void loop() {
	// check if a peripheral has been discovered
	//NRF_RADIO->TXPOWER = 8;
	BLEDevice peripheral = BLE.available();

	if (peripheral) {
		/*
		if (peripheral.hasLocalName() && peripheral.localName().equals("EggCallback"))
		{
			Serial.println(peripheral.rssi());
		}
		*/
		Serial.println(peripheral.rssi());
	}
}
