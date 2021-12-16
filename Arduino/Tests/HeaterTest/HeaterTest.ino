#include <ArduinoBLE.h>
#include "SensorTEMP.h"
#include "Arduino.h"
#include "Wire.h"
#include "HTS221.h"
#include "HTS.h"

#define BLE_UUID_HEATER_DATA_SERVICE     "f1111a7f-0000-41f9-a127-aedf35a799b3"
#define BLE_UUID_HEATER               "f1111a7f-0004-41f9-a127-aedf35a799b3"

BLEService heaterService(BLE_UUID_HEATER_DATA_SERVICE);
BLEUnsignedShortCharacteristic heaterCharacteristic(BLE_UUID_HEATER, BLERead | BLEWrite);

SensorTemp t;
SensorTemp t_in;

bool heatingActive = false;

uint32_t HEATING_TIME_MS = 5000;
uint32_t COOLING_DOWN_TIME_MS = 1000;
uint32_t MEASURING_DELAY_MS = 500;

float EXTERN_TEMP_OFFSET = -2.0f;
float EXTERN_HUMI_OFFSET = 0.0f;

float INTERN_TEMP_OFFSET = -2.5f;
float INTERN_HUMI_OFFSET = -0.0f;


void setup() {
	Serial.begin(9600);
	while (!Serial);

	t.setWire(&Wire);
	thread_sleep_for(1);
	t.init();

	t_in.setWire(&Wire1);
	thread_sleep_for(1);
	t_in.init();

	thread_sleep_for(100);

	setupBleMode();

	thread_sleep_for(1000);

	Serial.println("setup finished");
}

void loop() {
	float* vals = nullptr;

	if (!heatingActive) {

		vals = t.getSensorValues();
		Serial.printf("Extern Temp: %.2f | Humi: %.2f  -- | -- ", vals[0] + EXTERN_TEMP_OFFSET, vals[1] + EXTERN_HUMI_OFFSET);
	}

	vals = t_in.getSensorValues();
	Serial.printf("Intern Temp: %.2f | Humi: %.2f \n", vals[0] + INTERN_TEMP_OFFSET, vals[1] + INTERN_HUMI_OFFSET);

	BLE.poll();
	thread_sleep_for(MEASURING_DELAY_MS);
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

	heaterService.addCharacteristic(heaterCharacteristic);

	// add service
	BLE.addService(heaterService);

	heaterCharacteristic.setEventHandler(BLEWritten, onHeatMsg);

	heaterCharacteristic.writeValue(false);

	// start advertising
	BLE.advertise();

	return true;
}

static void onHeatMsg(BLEDevice central, BLECharacteristic characteristic) {
	const uint8_t* buf = characteristic.value();
	uint16_t time = buf[0] | buf[1] << 8;
	heat(time);
}

void heat(uint16_t time) {
	Serial.printf("Heating for: %u", time);
	if (!heatingActive) {
		heatingActive = true;
		t.enableHeater();
		thread_sleep_for(time * 1000);
		t.disableHeater();
		thread_sleep_for(COOLING_DOWN_TIME_MS);
		heatingActive = false;
	}
}
