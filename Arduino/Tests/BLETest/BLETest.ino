/*
  BLETest

  - A test to check functionality of BT module
  - Adaption of existing ArduinoBLE example


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Mobile Phone with NRFConnect App

  Created 10 30 2020
  By Johannes Brunner

  -----------------------------------------------------------------------------
  Callback LED
  Source: https://github.com/arduino-libraries/ArduinoBLE/blob/master/examples/Peripheral/CallbackLED/CallbackLED.ino

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED. The callback features of the
  library are used.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
	Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include "MemoryFree.h"

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED



void setup() {
	Serial.begin(9600);
	while (!Serial);

	pinMode(ledPin, OUTPUT); // use the LED pin as an output
}

void loop() {
	BLELocalDevice peripheral;
	//peripheral.debug(Serial);
	// poll for BLE events
	Serial.println(freeMemory());
	if (!peripheral.begin()) {
		Serial.println("starting BLE failed!");
		while (1);
	}
	peripheral.advertise();
	long starttime = millis();
	while (millis() < starttime + 1000) {
		BLE.poll();
	}
	delay(50);
	//BLE.stopAdvertise();
	peripheral.end();
	delay(50);
}


void blePeripheralConnectHandler(BLEDevice central) {
	// central connected event handler
	Serial.print("Connected event, central: ");
	Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
	// central disconnected event handler
	Serial.print("Disconnected event, central: ");
	Serial.println(central.address());
}

void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
	// central wrote new value to characteristic, update LED
	Serial.print("Characteristic event, written: ");

	if (switchCharacteristic.value()) {
		Serial.println("LED on");
		digitalWrite(ledPin, HIGH);
	}
	else {
		Serial.println("LED off");
		digitalWrite(ledPin, LOW);
		BLE.end();
		delay(1000000000);
	}
}
