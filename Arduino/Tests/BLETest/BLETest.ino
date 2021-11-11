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

#if (MBED_VERSION > MBED_ENCODE_VERSION(6, 2, 0))
#define BLE_NAMESPACE ble 
#else
#define BLE_NAMESPACE ble::vendor::cordio
#endif


#include <ArduinoBLE.h>
#include "MemoryFree.h"
//#include "mbed.h"
//#include "BLE.h"

#include <driver/CordioHCIDriver.h>



/*------------------------*/

using namespace mbed;       // Enable us to call mbed functions without "mbed::"

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED

//extern BLE_NAMESPACE::CordioHCIDriver& ble_cordio_get_hci_driver();
//extern BLE_NAMESPACE::CordioHCIDriver& x;

BLELocalDevice peripheral;


void setup() {
	Serial.begin(9600);
	while (!Serial);

	pinMode(ledPin, OUTPUT); // use the LED pin as an output

}

void loop() {
	//peripheral.debug(Serial);
	// poll for BLE events
	Serial.println(freeMemory());
	if (!peripheral.begin()) {
		Serial.println("starting BLE failed!");
		while (1);
	}

	beaconTest();

	/*
	peripheral.advertise();
	long starttime = millis();
	while (millis() < starttime + 1000) {
		BLE.poll();
	}
	delay(50);
	//BLE.stopAdvertise();
	peripheral.end();
	delay(50);
	*/
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

void beaconTest() {
	char* uuidByte = "c336aa38054bb0483b0ae75027061982";
	int _major = 1;
	int _minor = 1;
	int _tx = 127;

	byte data[25] = {
	0X4C,0x00, //setting for iBeacons
	0x02,0x15,
	uuidByte[0],
	uuidByte[1],
	uuidByte[2],
	uuidByte[3],
	uuidByte[4],
	uuidByte[5],
	uuidByte[6],
	uuidByte[7],
	uuidByte[8],
	uuidByte[9],
	uuidByte[10],
	uuidByte[11],
	uuidByte[12],
	uuidByte[13],
	uuidByte[14],
	uuidByte[15],
	0,
	_major,
	0,
	_minor,
	_tx
	};
	peripheral.setManufacturerData(data, 25);// AGGIUNGE IL MANUFACT
	peripheral.advertise();
}
