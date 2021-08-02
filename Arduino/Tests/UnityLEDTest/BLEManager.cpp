#include "BLEManager.h"

bool BLEManager::connected_ = false;
bool BLEManager::timeSet_ = false;
bool BLEManager::disconnected_ = false;
bool BLEManager::initialized_ = false;
uint32_t BLEManager::bleStartTime_ = 0;
uint32_t BLEManager::lastBLEInteractionTime = 0;

BLEFloatCharacteristic* BLEManager::batteryC;
BLEUnsignedLongCharacteristic* BLEManager::timeCold;
BLEByteCharacteristic* BLEManager::tableC;
BLEByteCharacteristic* BLEManager::intervalCold;
BLECharacteristic* BLEManager::sensorsCold;

BLECharacteristic* characteristics[6];
//BLEManager::characteristicDef BLEManager::defs[NUM_OF_CHARACTERISTICS];


void BLEManager::init()
{
	initServices();
	initCharacteristics();
}

void BLEManager::begin(measuring* currentSensorValues, uint8_t dataLength, float batteryPercentage, uint32_t freeSDSpace)
{
	if (!BLE.begin()) {
		DEBUG_PRINTLN("Failed to start BLE!");
		return;
	}

	isRunning_ = true;

	// set the local name peripheral advertises
	BLE.setLocalName("EggCallback");

	BLE.setAdvertisedService(*eggService);

	// add service
	BLE.addService(*eggService);

	initCallbacks();

	// Set Battery value
	batteryC->setValue(batteryPercentage);

	// Set Sensor values
	for (int i = 0; i < NUMBER_OF_SENSORS_FOR_APP; i++)
	{
		if (i < dataLength) {
			multiSensorData.values[i] = (float)currentSensorValues[i].value;
		}
	}
	sensorsCold->setValue(multiSensorData.bytes, sizeof multiSensorData.bytes);

	//toDo: Set free sd-memory value

	// start advertising
	BLE.advertise();

	DEBUG_PRINTLN(("Bluetooth device active, waiting for connections..."));
}

void BLEManager::initServices()
{
	eggService = new BLEService(serviceUUID);

	/*
	batteryC = new BLEFloatCharacteristic(batterySpaceUUID, BLERead);
	timeCold = new BLEUnsignedLongCharacteristic(timeUUID, BLERead | BLEWrite);
	tableC = new BLEByteCharacteristic(modesUUID, BLERead | BLEWrite);
	intervalCold = new BLEByteCharacteristic(intervalUUID, BLERead | BLEWrite);
	sensorsCold = new BLECharacteristic(sensorsUUID, BLERead, sizeof multiSensorData.bytes);

	eggService->addCharacteristic(*batteryC);
	eggService->addCharacteristic(*timeCold);
	eggService->addCharacteristic(*tableC);
	eggService->addCharacteristic(*intervalCold);
	eggService->addCharacteristic(*sensorsCold);
	*/
}

void BLEManager::initCharacteristics()
{
	for (int i = 0; i < NUMBER_OF_SENSORS_FOR_APP; i++) {
		characteristics[i] = new BLECharacteristic(defs[i].uuid, defs[i].properties, defs[i].size);
		eggService->addCharacteristic(*characteristics[i]);
	}

}

void BLEManager::addCharacteristic(char* uuid, uint8_t properties, uint8_t sizeInBytes)
{
	BLECharacteristic characteristicToAdd(uuid, properties, sizeInBytes);
	eggService->addCharacteristic(characteristicToAdd);
}


void BLEManager::initCallbacks()
{
	// assign event handlers for connected, disconnected to peripheral
	BLE.setEventHandler(BLEConnected, onConnect);
	BLE.setEventHandler(BLEDisconnected, onDisconnect);

	// assign event handlers for characteristics
	timeCold->setEventHandler(BLEWritten, onMessage);
	tableC->setEventHandler(BLEWritten, onMessage);
	intervalCold->setEventHandler(BLEWritten, onMessage);

	// set an initial value for the characteristics
	batteryC->setValue(0);
	timeCold->setValue(0);
	tableC->setValue(0);
	intervalCold->setValue(0);
	sensorsCold->setValue(multiSensorData.bytes, sizeof multiSensorData.bytes);
}

void BLEManager::onConnect(BLEDevice central) {
	// central connected event handler
	DEBUG_PRINT("Connected event, central: ");
	DEBUG_PRINTLN(central.address());
	connected_ = true;
}

void BLEManager::onDisconnect(BLEDevice central) {
	// central disconnected event handler
	DEBUG_PRINT("Disconnected event, central: ");
	DEBUG_PRINTLN(central.address());
	connected_ = false;
	disconnected_ = true;
}

void BLEManager::onMessage(BLEDevice central, BLECharacteristic characteristic)
{
	const char* id = characteristic.uuid();

	if (strcmp(id, timeUUID) == 0) {
		if (bleStartTime_ == 0) {
			bleStartTime_ = timeCold->value();
			DEBUG_PRINT("BLE received startTime value: ");
			DEBUG_PRINTLN(bleStartTime_);
			timeSet_ = true;
		}
	}

	else if (strcmp(id, modesUUID) == 0) {
		uint8_t tableToUse = tableC->value();
		DEBUG_PRINT("BLE received table value: ");
		DEBUG_PRINTLN(tableToUse);
		if (tableToUse == 1) {
			activeTable = sensorStateTable;
		}
		else if (tableToUse == 2) {
			activeTable = mixedStateTable;
		}
		else if (tableToUse == 3) {
			activeTable = audioStateTable;
		}
	}

	else if (strcmp(id, intervalUUID) == 0) {
		uint8_t numOfMeasures = intervalCold->value();
		DEBUG_PRINT("BLE received measures value: ");
		DEBUG_PRINTLN(numOfMeasures);
		for (int i = 0; i < 24; i++) {
			sensorStateTable[i].numOfMeasures = numOfMeasures;
			audioStateTable[i].numOfMeasures = numOfMeasures;
			mixedStateTable[i].numOfMeasures = numOfMeasures;
		}
	}


	else {
		DEBUG_PRINTLN("Error in BLE onMessage!");
	}
}

uint32_t BLEManager::getStartTime() {
	return bleStartTime_;
}

uint32_t BLEManager::getLastBLEInteractionTime()
{
	return lastBLEInteractionTime;
}

void BLEManager::run()
{
	BLE.poll();
}

void BLEManager::end()
{
	BLE.end();
	isRunning_ = false;
}

bool BLEManager::isRunning() {
	return isRunning_;

}

bool BLEManager::getConnectionState()
{
	return connected_;
}

bool BLEManager::isTimeSet()
{
	return timeSet_;
}

bool BLEManager::isStarted()
{
	return initialized_;
}

void BLEManager::setConnectionState(bool state)
{
	connected_ = state;
	disconnected_ = state;
}

