/*
  BLEConsumptionTest

  - A test for the unity asset ble example


  The circuit:
  * Board: Arduino Nano 33 BLE Sense

  Created 26 04 2021
  By Johannes Brunner

*/

#define LED_DEBUG
#define NUMBER_OF_SENSORS_FOR_APP 8

#include <ArduinoBLE.h>
#include "BLEManager.h"

uint8_t bytesToSend[NUMBER_OF_SENSORS_FOR_APP * sizeof(float) + NUMBER_OF_SENSORS_FOR_APP * sizeof(byte) + 4];

BLEService ledService("A9E90000-194C-4523-A473-5FDF36AA4D20"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("A9E90001-194C-4523-A473-5FDF36AA4D20", BLERead | BLEWrite);
BLEByteCharacteristic writeCharacteristic("A9E90002-194C-4523-A473-5FDF36AA4D20", BLEWrite);
BLECharacteristic sensorCharacteristics("A9E90003-194C-4523-A473-5FDF36AA4D20", BLENotify, NUMBER_OF_SENSORS_FOR_APP * 5 + 4);

uint32_t NOTIFY_INTERVAL_MS = 1000;
uint32_t lastUpdate = 0;
bool notifySwitch = false;

float sensorVals[NUMBER_OF_SENSORS_FOR_APP] = { 45.76, 34.23, 23.34, 67.34, 334.2323, 223.121212, 22323.4, 0.6 };
byte sensorIDs[NUMBER_OF_SENSORS_FOR_APP] = { 0, 1, 2,3, 4, 5, 6, 7 };
uint32_t unixTime = 162023853;

// Variables:

// toDo: apply the changes to the electronic egg operation

void setup() {
	delay(1000);

	while (!Serial);

	if (!BLE.begin()) {
		Serial.println("starting BLE failed!");
		//while (1);
	}

	// set the local name peripheral advertises
	BLE.setLocalName("LEDCallback");
	// set the UUID for the service this peripheral advertises
	BLE.setAdvertisedService(ledService);

	// add the characteristic to the service
	ledService.addCharacteristic(switchCharacteristic);

	ledService.addCharacteristic(writeCharacteristic);

	ledService.addCharacteristic(sensorCharacteristics);

	// add service
	BLE.addService(ledService);

	// assign event handlers for connected, disconnected to peripheral
	BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
	BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

	// assign event handlers for characteristic
	switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);
	// set an initial value for the characteristic

	writeCharacteristic.setEventHandler(BLEWritten, onRefreshCommand);


	switchCharacteristic.setValue(0);

	BLE.setDeviceName("ledbtn");
	BLE.setLocalName("ledbtn");


	// start advertising
	BLE.advertise();

	Serial.println(("Bluetooth device active, waiting for connections..."));
}

void loop() {


	BLE.poll();

	/*
	if (millis() > (lastUpdate + NOTIFY_INTERVAL_MS)) {

		for (int i = 0; i < NUMBER_OF_SENSORS_FOR_APP * 5; i += 5) {
			newBytes[i] = sensorIDs[i/5];
			byte* valBytes = (byte*)&sensorVals[i/5];
			for (int j = 0; j < sizeof(float); j++) {
				newBytes[i + (j + 1)] = ((byte*)&sensorVals[i / 5])[j];
			}
		}
		sensorCharacteristics.setValue(newBytes, sizeof newBytes);
		lastUpdate = millis();
	}
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
		digitalWrite(LED_BUILTIN, HIGH);
	}
	else {
		Serial.println("LED off");
		digitalWrite(LED_BUILTIN, LOW);
	}
}

void onRefreshCommand(BLEDevice central, BLECharacteristic characteristic) {
	refreshSensors();
};

void refreshSensors() {
	for (int i = 0; i < NUMBER_OF_SENSORS_FOR_APP; i++) {
		sensorVals[i] = getRndSensorValue();
	}

	// add unix timestamp to refreshed sensors
	for (int i = 0; i < 4; i++) {
		bytesToSend[i] = ((byte*)&unixTime)[i];
	}

	// add sensor values and ids before every float value
	for (int i = 4; i < NUMBER_OF_SENSORS_FOR_APP * 5 + 4; i += 5) {
		Serial.println(i);
		bytesToSend[i] = sensorIDs[(i - 4) / 5];
		for (int j = 0; j < sizeof(float); j++) {
			bytesToSend[i + (j + 1)] = ((byte*)&sensorVals[(i - 4) / 5])[j];
		}
	}

	for (int i = 0; i < 44; i++) {
		Serial.print(i);
		Serial.print(":");
		Serial.print(bytesToSend[i]);
		Serial.print(" ");
	}
	Serial.println();

	sensorCharacteristics.setValue(bytesToSend, sizeof bytesToSend);
}

float getRndSensorValue() {
	return random(1, 5000) / 100.0;
}
