#include <ArduinoBLE.h>
#include "SensorTEMP.h"
#include "Arduino.h"
#include "Wire.h"
#include "HTS221.h"
#include "HTS.h"


#define BLE_UUID_HEATER_DATA_SERVICE              "f1111a7f-0000-41f9-a127-aedf35a799b3"
#define BLE_UUID_TEMP_HUMI              "f1111a7f-0003-41f9-a127-aedf35a799b3"
#define BLE_UUID_TEMP_TEMP              "f1111a7f-0004-41f9-a127-aedf35a799b3"
#define BLE_UUID_TEMP_INT              "f1111a7f-0005-41f9-a127-aedf35a799b3"
#define BLE_UUID_HEATER               "f1111a7f-0004-41f9-a127-aedf35a799b3"


bool usingDebugBoard = true;

BLEService heaterService(BLE_UUID_HEATER_DATA_SERVICE);
BLEFloatCharacteristic tempChar(BLE_UUID_TEMP_TEMP, BLERead | BLENotify);
BLEFloatCharacteristic humiChar(BLE_UUID_TEMP_HUMI, BLERead | BLENotify);
BLEBoolCharacteristic heaterCharacteristic(BLE_UUID_HEATER, BLERead | BLEWrite);
SensorTemp t;


void setup() {
	Serial.begin(9600);
	//while ( !Serial );
	setupBleMode();

	if (usingDebugBoard) {
		t.setWire(&Wire1);
	}
	else {
		t.setWire(&Wire);
	}
	t.init();
}

void loop() {
	//float* values = t.getSensorValues();

	float values[2];
	values[0] = 0;
	values[1] = 0;

	//t.stop();

	tempChar.writeValue(values[0]);
	humiChar.writeValue(values[1]);

	Serial.print("Temp: ");
	Serial.print(values[0]);
	Serial.print("  |  ");
	Serial.print("Humi: ");
	Serial.println(values[1]);

	if (heaterCharacteristic.value() == 1) {
		//t.writeRegister(0x5F, 0x21, (uint8_t)64);
		t.enableHeater();
	}
	else {
		t.writeRegister(0x5F, 0x21, (uint8_t)0);
	}

	//thread_sleep_for(1);
	Serial.println(t.readRegister(0x5F, 0x21));

	BLE.poll();
	thread_sleep_for(100);
}

bool setupBleMode()
{
	if (!BLE.begin())
	{
		return false;
	}

	// set advertised local name and service UUID:
	BLE.setDeviceName("HeaterCallback");
	BLE.setLocalName("HeaterCallback");
	BLE.setAdvertisedService(heaterService);

	// BLE add characteristics
	heaterService.addCharacteristic(humiChar);
	heaterService.addCharacteristic(tempChar);
	heaterService.addCharacteristic(heaterCharacteristic);

	// add service
	BLE.addService(heaterService);

	heaterCharacteristic.setEventHandler(BLEWritten, onHeatMsg);

	// set the initial value for the characeristic:
	tempChar.writeValue(0.0f);
	humiChar.writeValue(0.0f);

	// start advertising
	BLE.advertise();

	return true;
}

static void onHeatMsg(BLEDevice central, BLECharacteristic characteristic) {
	SensorTemp t;

	if (characteristic.value()[0] == 1) {
		Serial.println("heater on");
		//t.writeRegister(0x5F, 0x21, (uint8_t)64);
	}
	else {
		Serial.println("heater off");
		//t.writeRegister(0x5F, 0x21, 0);
	}

}

