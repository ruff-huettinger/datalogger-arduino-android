#include "BLEManager.h"

bool BLEManager::connected_ = false;
bool BLEManager::timeSet_ = false;
bool BLEManager::disconnected_ = false;
bool BLEManager::initialized_ = false;
uint32_t BLEManager::bleStartTime_ = 0;
uint32_t BLEManager::lastBLEInteractionTime = 0;

BLECharacteristic* BLEManager::characteristics[6];


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

	BLE.advertise();

	DEBUG_PRINTLN(("Bluetooth device active, waiting for connections..."));
}

void BLEManager::initServices()
{
	eggService = new BLEService(serviceUUID);
}

void BLEManager::initCharacteristics()
{
	for (int i = 0; i < 6; i++) {
		characteristics[i] = new BLECharacteristic(defs[i].uuid, defs[i].properties, defs[i].size);
		eggService->addCharacteristic(*characteristics[i]);
	}

}

void BLEManager::initCallbacks()
{
	// assign event handlers for connected, disconnected to peripheral
	BLE.setEventHandler(BLEConnected, onConnect);
	BLE.setEventHandler(BLEDisconnected, onDisconnect);

	for (int i = 0; i < 6; i++) {
		if (defs[i].properties == BLEWrite) {
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

	if (strcmp(id, timeUUID) == 0) {
		DEBUG_PRINT("BLE received time value: ");
		if (bleStartTime_ == 0) {
			//bleStartTime_ = (uint32_t)characteristics[TIME]->value();
			DEBUG_PRINT("BLE received time value: ");
			DEBUG_PRINTLN(bleStartTime_);
			timeSet_ = true;
		}
	}

	else if (strcmp(id, modesUUID) == 0) {
		//uint8_t modes = (uint8_t)characteristics[MODES]->value();
		DEBUG_PRINTLN("BLE received modes value: ");
	}

	else if (strcmp(id, intervalUUID) == 0) {
		//uint8_t numOfMeasures = (uint8_t)characteristics[INTERVAL]->value();
		DEBUG_PRINTLN("BLE received interval value: ");
	}

	else if (strcmp(id, updateUUID) == 0) {
		refreshSensors();
	}

	else {
		DEBUG_PRINTLN("Error in BLE onMessage!");
	}
}


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

void BLEManager::refreshSensors()
{
	SensorManager sm;
	uint8_t numOfSensors = sm.getDataLength();
	uint8_t numOfBytesToSend = numOfSensors * 5 + 4;

	measuring* sensorData = new measuring[numOfSensors];
	sm.getSensorValues(sensorData);

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

