#pragma once

#include <ArduinoBLE.h>
#include "config.h"

/**
 * @file BLEManager.h
 *
 * - Handles the BLE-connection using ArduinoBLE-lib
 * - defines services and characteristics for the electronic egg app
 *
 * @author Johannes Brunner
 *
 */

 //toDo: add function to refresh sensor values in sensorsC via BLE (and display the actual values)
 //toDo: add characteristic for custom stateTable created by BLE-device (24 byte with hBT or hAudio or hSensor)
 //toDo: add characteristic for sd card space
 //toDo: reduce consumption after BLE(bug in BLE-core -> https://github.com/arduino-libraries/ArduinoBLE/issues/149)

#define NUM_OF_CHARACTERISTICS 6

class BLEManager
{

private:
	typedef struct characteristicDef {
		char* uuid;
		uint8_t properties;
		uint8_t size;
	} characteristicDef;

	static constexpr char* serviceUUID = "f1111a7f-0000-41f9-a127-aedf35a799b3";
	static constexpr char* timeUUID = "f1111a7f-0001-41f9-a127-aedf35a799b3";
	static constexpr char* modesUUID = "f1111a7f-0002-41f9-a127-aedf35a799b3";
	static constexpr char* intervalUUID = "f1111a7f-0003-41f9-a127-aedf35a799b3";
	static constexpr char* batterySpaceUUID = "f1111a7f-0004-41f9-a127-aedf35a799b3";
	static constexpr char* sensorsUUID = "f1111a7f-0005-41f9-a127-aedf35a799b3";
	static constexpr char* updateUUID = "f1111a7f-0006-41f9-a127-aedf35a799b3";

	/*
	* Characteristicts to implement:
	   1. Time Read Write (Set time and and see startup and compare) 4*byte
	   2. Mode Write 24*byte (hAudio, hSensor, hBLE)
	   3. Interval Write byte
	   4. Battery + Space Read 2 * float
	   5. Sensors Read Notify timestamp  (4byte) + (byte + float) * NUM_OF_SENSORS
	   6. Sensor Update Command Write Byte

	static constexpr characteristicDef timeC = { timeUUID, BLERead | BLEWrite, 4 };
	static constexpr characteristicDef modeC = { modesUUID, BLEWrite, 24 };
	static constexpr characteristicDef intervalC = { intervalUUID, BLEWrite, 1 };
	static constexpr characteristicDef batterySpaceC = { batterySpaceUUID, BLERead, 8 };
	static constexpr characteristicDef sensorsC = { sensorsUUID, BLERead | BLENotify, 4 + 5 * NUMBER_OF_SENSORS_FOR_APP };
	static constexpr characteristicDef updateC = { updateUUID, BLEWrite, 1 };
	*/

	static constexpr characteristicDef defs[NUM_OF_CHARACTERISTICS] = {
		{ timeUUID, BLERead | BLEWrite, 4 },
		{ modesUUID, BLEWrite, 24 },
		{ intervalUUID, BLEWrite, 1 },
		{ batterySpaceUUID, BLERead, 8 },
		{ sensorsUUID, BLERead | BLENotify, 4 + 5 * NUMBER_OF_SENSORS_FOR_APP },
		{ updateUUID, BLEWrite, 1 }
	};

	static BLECharacteristic* characteristics[NUM_OF_CHARACTERISTICS];

	BLEService* eggService; // create service

	// create switch characteristic and allow remote device to read and write
	static BLEFloatCharacteristic* batteryC;
	static BLEUnsignedLongCharacteristic* timeCold;
	static BLEByteCharacteristic* tableC;
	static BLEByteCharacteristic* intervalCold;
	static BLEByteCharacteristic* startUpC;
	static BLECharacteristic* sensorsCold;


	static bool connected_;
	static bool disconnected_;
	static bool timeSet_;
	static bool initialized_;
	static uint32_t lastBLEInteractionTime;
	static uint32_t bleStartTime_;

	bool isRunning_ = false;


	static void onConnect(BLEDevice central);
	static void onDisconnect(BLEDevice central);
	static void onMessage(BLEDevice central, BLECharacteristic characteristic);

	void initCallbacks();
	void initServices();
	void initCharacteristics();

	void addCharacteristic(char* uuid, uint8_t size, uint8_t properties);

	union multi_sensor_data
	{
		struct __attribute__((packed))
		{
			float values[NUMBER_OF_SENSORS_FOR_APP];
		};
		uint8_t bytes[NUMBER_OF_SENSORS_FOR_APP * sizeof(float)];
	};

	union multi_sensor_data multiSensorData;

public:

	void init();
	void begin(measuring* currentSensorValues, uint8_t dataLength, float batteryPercentage, uint32_t freeSDSpace = 0);
	void run();
	void end();
	bool isRunning();
	bool getConnectionState();
	bool isTimeSet();
	bool isStarted();
	void setConnectionState(bool state);
	uint32_t getStartTime();
	uint32_t getLastBLEInteractionTime();

	BLEManager() {};
	~BLEManager() {};
};

