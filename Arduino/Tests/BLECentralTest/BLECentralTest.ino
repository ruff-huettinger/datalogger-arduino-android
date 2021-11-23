/*
  Scan

  This example scans for BLE peripherals and prints out their advertising details:
  address, local name, advertised service UUID's.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
	Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  This example code is in the public domain.
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