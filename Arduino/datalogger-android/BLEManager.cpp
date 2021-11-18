#include "BLEManager.h"

bool BLEManager::connected_ = false;
bool BLEManager::timeSet_ = false;
bool BLEManager::disconnected_ = false;
bool BLEManager::initialized_ = false;
uint32_t BLEManager::bleStartTime_ = 0;
uint32_t BLEManager::lastBLEInteractionTime = 0;
byte BLEManager::updatedTable[24] = { 0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0 };
byte BLEManager::updatedInterval = 1;

BLECharacteristic* BLEManager::characteristics[NUM_OF_CHARACTERISTICS];


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

	BLE.setAdvertisedService(*eggService);

	// add service
	BLE.addService(*eggService);

	initCallbacks();

	BLE.advertise();

	updateBatSpaceValues(batteryPercentage, cardSize, freeSDSpace);

	updateModesValue();

	byte startedValue[1] = { isStarted() };

	if (!isStarted()) {
		startedValue[0] = fm->checkSDIntegrity();
	}
	characteristics[STARTED]->setValue(startedValue, 1);

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

	characteristics[BATSPACE]->setValue(bytesToSend, 12);
}

void BLEManager::updateModesValue()
{
	byte modesBefore[24] = {};
	for (int i = 0; i < 24; i++) {
		modesBefore[i] = (byte)activeTable[i].hourMode;
	}

	byte modesAfter[12] = {};

	modeToBLE(modesBefore, modesAfter);

	characteristics[MODES]->setValue(modesAfter, 12);

	byte intervalToSend[1] = { activeTable[0].numOfMeasures };
	characteristics[INTERVAL]->setValue(intervalToSend, 1);
}

void BLEManager::initServices()
{
	eggService = new BLEService(serviceUUID);
}


void BLEManager::initCharacteristics()
{
	for (int i = 0; i < NUM_OF_CHARACTERISTICS; i++) {
		characteristics[i] = new BLECharacteristic(defs[i].uuid, defs[i].properties, defs[i].size);
		eggService->addCharacteristic(*characteristics[i]);
	}

}

void BLEManager::initCallbacks()
{
	// assign event handlers for connected, disconnected to peripheral
	BLE.setEventHandler(BLEConnected, onConnect);
	BLE.setEventHandler(BLEDisconnected, onDisconnect);

	for (int i = 0; i < NUM_OF_CHARACTERISTICS; i++) {
		if (defs[i].properties == BLEWrite || (BLERead | BLEWrite)) {
			characteristics[i]->setEventHandler(BLEWritten, onMessage);
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

	if (strcmp(id, timeUUID) == 0) {
		if (bleStartTime_ == 0) {
			bleStartTime_ = *(long*)characteristics[TIME]->value();
			DEBUG_PRINT("BLE received time value: ");
			DEBUG_PRINTLN(bleStartTime_);
			timeSet_ = true;
		}
	}

	else if (strcmp(id, modesUUID) == 0) {
		DEBUG_PRINTLN("Received new modes table");

		byte* arr = (byte*)characteristics[MODES]->value();

		bleToMode(updatedTable, arr);
	}

	else if (strcmp(id, intervalUUID) == 0) {
		DEBUG_PRINT("BLE received interval value: ");
		updatedInterval = characteristics[INTERVAL]->value()[0];
		DEBUG_PRINTLN(updatedInterval);
	}

	else if (strcmp(id, updateUUID) == 0) {
		refreshSensors();
	}

	else if (strcmp(id, startedUUID) == 0) {
		byte id = (characteristics[STARTED]->value())[0];
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
	characteristics[SENSORS]->setValue(bytesToSend, numOfBytesToSend);
	delete[] sensorData;
	delete[] bytesToSend;
}

uint32_t BLEManager::getStartTime() {
	return bleStartTime_;
}

uint32_t BLEManager::getLastBLEInteractionTime()
{
	return lastBLEInteractionTime;
}

byte* BLEManager::getUpdatedTable()
{
	return updatedTable;
}

byte BLEManager::getUpdatedInterval() {
	return updatedInterval;
}

void BLEManager::setFileManager(FileManager* fmNew)
{
	fm = fmNew;
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

void BLEManager::bleToMode(byte* large, byte* small)
{
	for (int i = 0; i < 12; i++) {
		byte fullNum = small[i];
		byte firstValue = fullNum % 3;
		byte secondValue = (fullNum / 3) % 3;
		large[i * 2] = firstValue;
		large[i * 2 + 1] = secondValue;
	}
}

void BLEManager::modeToBLE(byte* large, byte* small)
{
	for (int i = 0; i < 24; i = i + 2) {
		byte firstValue = (byte)large[i];
		byte secondValue = (byte)large[i + 1];
		small[i / 2] = (byte)(firstValue + secondValue * 3);
	}
}


/*
void BLEManager::refreshTilt() {
	Acceleromter acc;
	byte arr[16];
	long x = 76;
	arr[0] = x;

	float pitch = acc.getPitch();
	float roll = acc.getRoll();
	float yaw = acc.getYaw();

	for (int j = 4; j < sizeof(float) + 4; j++) {
		arr[j] = ((byte*)&pitch)[j];
	}

	long l = *(long*)&pitch;

	arr[4] = l & 0x00FF;
	arr[5] = (l >> 8) & 0x00FF;
	arr[6] = (l >> 16) & 0x00FF;
	arr[7] = l >> 24;

	l = *(long*)&roll;

	arr[8] = l & 0x00FF;
	arr[9] = (l >> 8) & 0x00FF;
	arr[10] = (l >> 16) & 0x00FF;
	arr[11] = l >> 24;

	l = *(long*)&yaw;

	arr[12] = l & 0x00FF;
	arr[13] = (l >> 8) & 0x00FF;
	arr[14] = (l >> 16) & 0x00FF;
	arr[15] = l >> 24;

	characteristics[SENSORS]->setValue(arr, 16);
}

void BLEManager::refreshQuat() {
	Acceleromter acc;
	byte arr[20];
	long x = 76;
	arr[0] = x;

	float* q;
	q = acc.getQuaternion();


	long l = *(long*)&q[0];

	arr[4] = l & 0x00FF;
	arr[5] = (l >> 8) & 0x00FF;
	arr[6] = (l >> 16) & 0x00FF;
	arr[7] = l >> 24;

	l = *(long*)&q[1];

	arr[8] = l & 0x00FF;
	arr[9] = (l >> 8) & 0x00FF;
	arr[10] = (l >> 16) & 0x00FF;
	arr[11] = l >> 24;

	l = *(long*)&q[2];

	arr[12] = l & 0x00FF;
	arr[13] = (l >> 8) & 0x00FF;
	arr[14] = (l >> 16) & 0x00FF;
	arr[15] = l >> 24;

	l = *(long*)&q[3];

	arr[16] = l & 0x00FF;
	arr[17] = (l >> 8) & 0x00FF;
	arr[18] = (l >> 16) & 0x00FF;
	arr[19] = l >> 24;

	characteristics[SENSORS]->setValue(arr, 20);
}
*/

