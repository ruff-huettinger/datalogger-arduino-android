#pragma once

#include <ArduinoBLE.h>
#include "config.h"
#include "SensorManager.h"
#include "FileManager.h"


/**
 * @file BLEManager.h
 *
 * - handles the BLE-connection using ArduinoBLE-lib
 * - defines services and characteristics for the electronic egg app
 * - Note: There is a bug in  ArduinoBLE-lib (https://github.com/arduino-libraries/ArduinoBLE/issues/149)
 *         which causes the power consumption to be increased by 0.3mA after using BLE
 * @author Johannes Brunner
 *
 */


#define NUM_OF_CHARACTERISTICS 7

class BLEManager
{

private:
	typedef struct characteristicDef {
		char* uuid;
		uint8_t properties;
		uint8_t size;
	} characteristicDef;

	static constexpr char* SERVICE_UUID = "f1111a7f-0000-41f9-a127-aedf35a799b3";
	static constexpr char* TIME_UUID = "f1111a7f-0001-41f9-a127-aedf35a799b3";
	static constexpr char* MODES_UUID = "f1111a7f-0002-41f9-a127-aedf35a799b3";
	static constexpr char* INTERVAL_UUID = "f1111a7f-0003-41f9-a127-aedf35a799b3";
	static constexpr char* BATTERY_SPACE_UUID = "f1111a7f-0004-41f9-a127-aedf35a799b3";
	static constexpr char* SENSORS_UUID = "f1111a7f-0005-41f9-a127-aedf35a799b3";
	static constexpr char* UPDATE_UUID = "f1111a7f-0006-41f9-a127-aedf35a799b3";
	static constexpr char* STARTED_UUID = "f1111a7f-0007-41f9-a127-aedf35a799b3";

	const characteristicDef defs_[NUM_OF_CHARACTERISTICS] = {
		{ TIME_UUID, BLERead | BLEWrite, 4 },
		{ MODES_UUID, BLERead | BLEWrite, 12 }, // 1 byte for two hours
		{ INTERVAL_UUID, BLERead | BLEWrite, 1 },
		{ BATTERY_SPACE_UUID, BLERead | BLEWrite, 12 },
		{ SENSORS_UUID, BLERead | BLENotify, 4 + 5 * NUMBER_OF_SENSORS_FOR_APP },
		{ UPDATE_UUID, BLEWrite, 1 },
		{ STARTED_UUID, BLERead | BLEWrite, 1 }
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

	static BLECharacteristic* characteristics_[NUM_OF_CHARACTERISTICS];

	BLEService* service_; // create service

	static uint8_t connected_;
	static uint8_t disconnected_;
	static uint8_t timeSet_;
	static uint8_t initialized_;
	uint8_t isRunning_ = false;
	static uint32_t lastBLEInteractionTime_;
	static uint32_t bleStartTime_;

	static uint8_t updatedTable_[24];
	static uint8_t updatedInterval_;

	FileManager* fm_;

	void initCallbacks();
	void initServices();
	void initCharacteristics();

	static void onConnect(BLEDevice central);
	static void onDisconnect(BLEDevice central);
	static void onMessage(BLEDevice central, BLECharacteristic characteristic);
	static void refreshSensors();
	static void modeToBLE(uint8_t* large, uint8_t* small);
	static void bleToMode(uint8_t* large, uint8_t* small);

public:
	void init();
	void begin(measuring* currentSensorValues, uint8_t dataLength, float batteryPercentage, uint32_t cardSize = 0, uint32_t freeSDSpace = 0);
	void updateBatSpaceValues(float batteryPercentage, uint32_t cardSize, uint32_t writtenBytes);
	void updateModesValue();
	void run();
	void end();
	uint8_t isRunning();
	uint8_t getConnectionState();
	uint8_t isTimeSet();
	uint8_t isStarted();
	void setConnectionState(uint8_t state);
	uint32_t getStartTime();
	uint32_t getLastBLEInteractionTime();
	uint8_t* getUpdatedTable();
	uint8_t getUpdatedInterval();
	void setFileManager(FileManager* fmNew);

	BLEManager() {};
	~BLEManager() {};
};

