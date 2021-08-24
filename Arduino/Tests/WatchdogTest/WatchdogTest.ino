#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

BLELocalDevice peripheral;

int ledState = LOW;

// watchdog timeout in seconds
int wdt = 3;

void setup() {
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	if (!peripheral.begin()) {
		Serial.println("starting BLE failed!");
		while (1);
	}

	peripheral.setLocalName("TestCallback");
	peripheral.setAdvertisedService(ledService);
	ledService.addCharacteristic(switchCharacteristic);
	peripheral.addService(ledService);
	peripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
	peripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
	switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);
	switchCharacteristic.setValue(0);
	peripheral.advertise();

	delay(2000);
	Serial.println("booting up finished -> BLE UP");
}

void loop() {
	if (peripheral.connected()) {
		peripheral.stopAdvertise();
	}
	else {
		peripheral.advertise();
	}
	BLE.poll();
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
		//Configure WDT.
	Serial.println("Watchdoog Written");
	NRF_WDT->CONFIG = 0x01;             // Configure WDT to run when CPU is asleep
	NRF_WDT->CRV = wdt * 32768 + 1;  // set timeout
	//NRF_WDT->RREN = 0x01;             // Enable the RR[0] reload register
	NRF_WDT->TASKS_START = 1;                // Start WDT
}