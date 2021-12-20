#include "BLEManager.h"

uint8_t BLEManager::connected_ = false;
uint8_t BLEManager::timeSet_ = false;
uint8_t BLEManager::disconnected_ = false;
uint8_t BLEManager::initialized_ = false;
uint32_t BLEManager::bleStartTime_ = 0;
uint32_t BLEManager::lastBLEInteractionTime_ = 0;
uint8_t BLEManager::updatedTable_[24] = { 0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0 };
uint8_t BLEManager::updatedInterval_ = 1;
BLECharacteristic* BLEManager::characteristics_[NUM_OF_CHARACTERISTICS];


void BLEManager::init()
{
	initServices();
	initCharacteristics();
}

void BLEManager::begin(measuring* currentSensorValues, uint8_t dataLength, float batteryPercentage, uint32_t cardSize, uint32_t freeSDSpace)
{
	if (!BLE.begin()) {
		DEBUG_PRINTLN("Failed to start BLE!");
		return;
	}

	isRunning_ = true;

	// set the local name peripheral advertises
	BLE.setLocalName("EggCallback");

	BLE.setAdvertisedService(*service_);

	// add service
	BLE.addService(*service_);

	initCallbacks();

	BLE.advertise();

	updateBatSpaceValues(batteryPercentage, cardSize, freeSDSpace);

	updateModesValue();

	byte startedValue[1] = { isStarted() };

	if (!isStarted()) {
		startedValue[0] = fm_->checkSDIntegrity();
	}
	characteristics_[STARTED]->setValue(startedValue, 1);

	DEBUG_PRINTLN(("Bluetooth device active, waiting for connections..."));
}

void BLEManager::updateBatSpaceValues(float batteryPercentage, uint32_t cardSize, uint32_t writtenBytes)
{
	byte bytesToSend[12];

	for (int h = 0; h < sizeof(float); h++) {
		bytesToSend[h] = ((byte*)&batteryPercentage)[h];
		bytesToSend[h + 4] = ((byte*)&cardSize)[h];
		bytesToSend[h + 8] = ((byte*)&writtenBytes)[h];
	}

	characteristics_[BATSPACE]->setValue(bytesToSend, 12);
}

void BLEManager::updateModesValue()
{
	byte modesBefore[24] = {};
	for (int i = 0; i < 24; i++) {
		modesBefore[i] = (byte)activeTable[i].hourMode;
	}

	byte modesAfter[12] = {};

	modeToBLE(modesBefore, modesAfter);

	characteristics_[MODES]->setValue(modesAfter, 12);

	byte intervalToSend[1] = { activeTable[0].numOfMeasures };
	characteristics_[INTERVAL]->setValue(intervalToSend, 1);
}

void BLEManager::initServices()
{
	service_ = new BLEService(SERVICE_UUID);
}


void BLEManager::initCharacteristics()
{
	for (int i = 0; i < NUM_OF_CHARACTERISTICS; i++) {
		characteristics_[i] = new BLECharacteristic(defs_[i].uuid, defs_[i].properties, defs_[i].size);
		service_->addCharacteristic(*characteristics_[i]);
	}

}

void BLEManager::initCallbacks()
{
	// assign event handlers for connected, disconnected to peripheral
	BLE.setEventHandler(BLEConnected, onConnect);
	BLE.setEventHandler(BLEDisconnected, onDisconnect);

	for (int i = 0; i < NUM_OF_CHARACTERISTICS; i++) {
		if (defs_[i].properties == BLEWrite || (BLERead | BLEWrite)) {
			characteristics_[i]->setEventHandler(BLEWritten, onMessage);
		}
	}

	//characteristics[SENSORS]->setValue(getSensorValues(), 40);
}

void BLEManager::onConnect(BLEDevice central) {
	// central connected event handler
	LOGMEMORY;
	DEBUG_PRINT("Connected event, central: ");
	DEBUG_PRINTLN(central.address());
	connected_ = true;
}

void BLEManager::onDisconnect(BLEDevice central) {
	// central disconnected event handler
	LOGMEMORY;
	DEBUG_PRINT("Disconnected event, central: ");
	DEBUG_PRINTLN(central.address());
	connected_ = false;
	disconnected_ = true;
}

void BLEManager::onMessage(BLEDevice central, BLECharacteristic characteristic)
{
	const char* id = characteristic.uuid();

	LOGMEMORY;
	DEBUG_PRINTLN(id);

	if (strcmp(id, TIME_UUID) == 0) {
		if (bleStartTime_ == 0) {
			bleStartTime_ = *(long*)characteristics_[TIME]->value();
			DEBUG_PRINT("BLE received time value: ");
			DEBUG_PRINTLN(bleStartTime_);
			timeSet_ = true;
		}
	}

	else if (strcmp(id, MODES_UUID) == 0) {
		DEBUG_PRINTLN("Received new modes table");

		byte* arr = (byte*)characteristics_[MODES]->value();

		bleToMode(updatedTable_, arr);
	}

	else if (strcmp(id, INTERVAL_UUID) == 0) {
		DEBUG_PRINT("BLE received interval value: ");
		updatedInterval_ = characteristics_[INTERVAL]->value()[0];
		DEBUG_PRINTLN(updatedInterval_);
	}

	else if (strcmp(id, UPDATE_UUID) == 0) {
		refreshSensors();
	}

	else if (strcmp(id, STARTED_UUID) == 0) {
		byte id = (characteristics_[STARTED]->value())[0];
		DEBUG_PRINT(id);
		if (id == 1) {
			DEBUG_PRINTLN("Received start cmd");
			initialized_ = true;
		}
		else if (id == 2) {
			DEBUG_PRINTLN("Received wdt cmd");
			NRF_WDT->CONFIG = 0x01;             // Configure WDT to run when CPU is asleep
			NRF_WDT->CRV = 1 * 32768 + 1;  // set timeout
			//NRF_WDT->RREN = 0x01;             // Enable the RR[0] reload register
			NRF_WDT->TASKS_START = 1;
		}
	}

	else {
		DEBUG_PRINTLN("Error in BLE onMessage!");
	}
}


void BLEManager::refreshSensors()
{
	SensorManager sm;

	if (sm.getMeasurementActive()) {
		DEBUG_PRINTLN("measurement blocked");
		return;
	}

	uint8_t numOfSensors = sm.getDataLength();
	uint8_t numOfBytesToSend = numOfSensors * 5 + 4;

	measuring* sensorData = new measuring[numOfSensors];

	sm.setMeasurementActive(true);
	sm.getSensorValues(sensorData);
	sm.setMeasurementActive(false);

	byte* bytesToSend = new byte[numOfBytesToSend];

	uint64_t unixTime = 343421;

	// add timestamp in first 4 bytes
	for (int i = 0; i < 4; i++) {
		bytesToSend[i] = ((byte*)&unixTime)[i];
	}

	// add sensor values as float (4 bytes) and ids before every float value
	for (int i = 4; i < numOfBytesToSend; i += 5) {
		bytesToSend[i] = sensorData[(i - 4) / 5].id;
		for (int j = 0; j < sizeof(float); j++) {
			bytesToSend[i + (j + 1)] = ((byte*)&sensorData[(i - 4) / 5].value)[j];
		}
	}
	characteristics_[SENSORS]->setValue(bytesToSend, numOfBytesToSend);
	delete[] sensorData;
	delete[] bytesToSend;
}

uint32_t BLEManager::getStartTime() {
	return bleStartTime_;
}

uint32_t BLEManager::getLastBLEInteractionTime()
{
	return lastBLEInteractionTime_;
}

uint8_t* BLEManager::getUpdatedTable()
{
	return updatedTable_;
}

uint8_t BLEManager::getUpdatedInterval() {
	return updatedInterval_;
}

void BLEManager::setFileManager(FileManager* fmNew)
{
	fm_ = fmNew;
}


void BLEManager::run()
{
	NRF_RADIO->TXPOWER = 8;
	BLE.poll();
}

void BLEManager::end()
{
	BLE.end();
	isRunning_ = false;
}

uint8_t BLEManager::isRunning() {
	return isRunning_;

}

uint8_t BLEManager::getConnectionState()
{
	return connected_;
}

uint8_t BLEManager::isTimeSet()
{
	return timeSet_;
}

uint8_t BLEManager::isStarted()
{
	return initialized_;
}

void BLEManager::setConnectionState(uint8_t state)
{
	connected_ = state;
	disconnected_ = state;
}

void BLEManager::bleToMode(uint8_t* large, uint8_t* small)
{
	for (int i = 0; i < 12; i++) {
		byte fullNum = small[i];
		byte firstValue = fullNum % 3;
		byte secondValue = (fullNum / 3) % 3;
		large[i * 2] = firstValue;
		large[i * 2 + 1] = secondValue;
	}
}

void BLEManager::modeToBLE(uint8_t* large, uint8_t* small)
{
	for (int i = 0; i < 24; i = i + 2) {
		byte firstValue = (byte)large[i];
		byte secondValue = (byte)large[i + 1];
		small[i / 2] = (byte)(firstValue + secondValue * 3);
	}
}