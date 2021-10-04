#pragma once

#include <ArduinoBLE.h>
#include "config.h"
#include "SensorManager.h"
#include "FileManager.h"


/**
 * @file BLEManager.h
 *
 * - Handles the BLE-connection using ArduinoBLE-lib
 * - defines services and characteristics for the electronic egg app
 *
 * @author Johannes Brunner
 *
 */

 //toDo: add characteristic for custom stateTable created by BLE-device (24 byte with hBT or hAudio or hSensor)
 //toDo: add characteristic for sd card space
 //toDo: reduce consumption after BLE(bug in BLE-core -> https://github.com/arduino-libraries/ArduinoBLE/issues/149)

#define NUM_OF_CHARACTERISTICS 7

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
	static constexpr char* startedUUID = "f1111a7f-0007-41f9-a127-aedf35a799b3";

	const characteristicDef defs[NUM_OF_CHARACTERISTICS] = {
		{ timeUUID, BLERead | BLEWrite, 4 },
		{ modesUUID, BLERead | BLEWrite, 12 }, // 1 byte for two hours
		{ intervalUUID, BLERead | BLEWrite, 1 },
		{ batterySpaceUUID, BLERead | BLEWrite, 12 },
		{ sensorsUUID, BLERead | BLENotify, 4 + 5 * NUMBER_OF_SENSORS_FOR_APP },
		{ updateUUID, BLEWrite, 1 },
		{ startedUUID, BLERead | BLEWrite, 1 }
	};

	enum characteristicIDs
	{
		TIME,
		MODES,
		INTERVAL,
		BATSPACE,
		SENSORS,
		UPDATE,
		STARTED
	};

	static BLECharacteristic* characteristics[NUM_OF_CHARACTERISTICS];

	BLEService* eggService; // create service

	static bool connected_;
	static bool disconnected_;
	static bool timeSet_;
	static bool initialized_;
	static uint32_t lastBLEInteractionTime;
	static uint32_t bleStartTime_;

	bool isRunning_ = false;

	static byte updatedTable[24];
	static byte updatedInterval;

	static void onConnect(BLEDevice central);
	static void onDisconnect(BLEDevice central);
	static void onMessage(BLEDevice central, BLECharacteristic characteristic);
	static void refreshSensors();

	void initCallbacks();
	void initServices();
	void initCharacteristics();
	static void modeToBLE(byte* large, byte* small);
	static void bleToMode(byte* large, byte* small);

	FileManager* fm;

public:
	void init();
	void begin(measuring* currentSensorValues, uint8_t dataLength, float batteryPercentage, uint32_t cardSize = 0, uint32_t freeSDSpace = 0);
	void updateBatSpaceValues(float batteryPercentage, uint32_t cardSize, uint32_t writtenBytes);
	void updateModesValue();
	void run();
	void end();
	bool isRunning();
	bool getConnectionState();
	bool isTimeSet();
	bool isStarted();
	void setConnectionState(bool state);
	uint32_t getStartTime();
	uint32_t getLastBLEInteractionTime();
	void updateTable();
	byte* getUpdatedTable();
	byte getUpdatedInterval();

	void setFileManager(FileManager* fmNew);


	BLEManager() {};
	~BLEManager() {};
};

