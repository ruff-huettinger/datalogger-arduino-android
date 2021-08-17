#include "SensorManager.h"

bool SensorManager::measurementActive = false;

void SensorManager::getSensorValues(measuring* arr)
{
	//go over the struct-array slots and fill them if sensor is in use
	uint8_t slotToFill = 0;

	// --- IMU Sensor:
	// fill 9 slots of the struct-array

#if defined(USING_IMU_ACC) || defined(USING_IMU_GYRO) || defined(USING_IMU_MAG)
	SensorMotion imu;
	imu.init();

#ifdef USING_IMU_ACC
	arr[slotToFill].valueName = "accX";
	arr[slotToFill].id = ACCX;
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;

	arr[slotToFill].valueName = "accY";
	arr[slotToFill].id = ACCY;
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;

	arr[slotToFill].valueName = "accZ";
	arr[slotToFill].id = ACCZ;
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_IMU_ACC
#ifdef USING_IMU_GYRO
	arr[slotToFill].valueName = "gyroX";
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;

	arr[slotToFill].valueName = "gyroY";
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;

	arr[slotToFill].valueName = "gyroZ";
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_IMU_GYRO
#ifdef USING_IMU_MAG
	arr[slotToFill].valueName = "magX";
	arr[slotToFill].id = MAGX;
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;

	arr[slotToFill].valueName = "magY";
	arr[slotToFill].id = MAGY;
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;

	arr[slotToFill].valueName = "magZ";
	arr[slotToFill].id = MAGZ;
	imu.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_IMU_MAG
	imu.stop();
#endif // usingIMU

	// --- Temperature and Humidity Sensor:
	// fill two slots of the struct-array

#if defined (USING_HTS_TEMP) || defined(USING_HTS_HUMI)  
#if defined (USING_SHTC3)
	SensorSHTC3 hts;
#else
	SensorTemp hts;
#endif
#ifdef DEBUG_BOARD
	hts.setWire(&Wire1);
#else
	hts.setWire(&Wire);
#endif // DEBUG_BOARD

	hts.init();
#ifdef USING_HTS_TEMP
	arr[slotToFill].valueName = "temp";
	arr[slotToFill].id = TEMP;
	hts.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_HTS_TEMP
#ifdef USING_HTS_HUMI
	arr[slotToFill].valueName = "humi";
	arr[slotToFill].id = HUMI;
	hts.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_HTS_HUMI
	hts.stop();
#endif // usingHTS

	// --- Light and Proximity Sensor:
	// fill two slots of the struct-array

#if defined (USING_APDS_PROX) || defined (USING_APDS_LIGHT)
	SensorLight apds;
	apds.init();
#ifdef USING_APDS_PROX
	arr[slotToFill].valueName = "prox";
	apds.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_APDS_PROX
#ifdef USING_APDS_LIGHT
	arr[slotToFill].valueName = "light1";
	arr[slotToFill].id = LIGHT;
	apds.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_APDS_LIGHT
	apds.stop();
#endif // usingAPDS

	// --- Analog Light Sensor in egg shell:
	// fill two slots of the struct-array

#ifdef USING_ANALOG_LIGHT
	SensorLightAnalog ana;
	arr[slotToFill].valueName = "light2";
	arr[slotToFill].id = LIGHTANAONE;
	ana.getSensorValue(&arr[slotToFill]);
	slotToFill++;
	arr[slotToFill].valueName = "light3";
	arr[slotToFill].id = LIGHTANATWO;
	ana.getSensorValue(&arr[slotToFill]);
	slotToFill++;
#endif // USING_ANALOG_LIGHT

	// --- Battery:
	// fill one slots of the struct-array

#ifdef USING_BATTERY_SENSOR
	PowerManager pm;
	arr[slotToFill].sensorName = "BAT";
	arr[slotToFill].valueName = "bat";
	arr[slotToFill].id = BATSENS;
	arr[slotToFill].value = pm.getAnalogAvg();
	slotToFill++;
#endif // USING_BATTERY_SENSOR

#ifdef USING_HTS_INTERN
	SensorTemp htsi;
	htsi.setWire(&Wire1);
	htsi.init();
	arr[slotToFill].valueName = "temp";
	arr[slotToFill].id = INTTEMP;
	htsi.getSensorValue(&arr[slotToFill]);
	slotToFill++;
	arr[slotToFill].valueName = "humi";
	arr[slotToFill].id = INTHUMI;
	htsi.getSensorValue(&arr[slotToFill]);
	htsi.stop();
	slotToFill++;
#endif // USING_HTS_INTERN
}


uint8_t SensorManager::getDataLength() {

	uint8_t numOfMeasurings = 0;

#ifdef USING_IMU_ACC
	numOfMeasurings += 3;
#endif // USING_IMU_ACC
#ifdef USING_IMU_GYRO
	numOfMeasurings += 3;
#endif // USING_IMU_GYRO
#ifdef USING_IMU_MAG
	numOfMeasurings += 3;
#endif // USING_IMU_MAG
#ifdef USING_HTS_TEMP
	numOfMeasurings += 1;
#endif // USING_HTS_TEMP
#ifdef USING_HTS_HUMI
	numOfMeasurings += 1;
#endif // USING_HTS_HUMI
#ifdef USING_APDS_LIGHT
	numOfMeasurings += 1;
#endif // USING_APDS_LIGHT
#ifdef USING_APDS_PROX
	numOfMeasurings += 1;
#endif // USING_APDS_PROX
#ifdef USING_ANALOG_LIGHT
	numOfMeasurings += 2;
#endif // USING_ANALOG_LIGHT
#ifdef USING_BATTERY_SENSOR
	numOfMeasurings += 1;
#endif // USING_BATTERY_SENSOR
#ifdef USING_HTS_INTERN
	numOfMeasurings += 2;
#endif // USING_HTS_INTERN
	return numOfMeasurings;
}

void SensorManager::printSensorValues(measuring* dataToPrint, uint8_t numOfSensors)
{
	for (int i = 0; i < numOfSensors; i++) {
		DEBUG_PRINT(dataToPrint[i].sensorName);
		if (i != (numOfSensors - 1))
			DEBUG_PRINT("\t");
		else {
			DEBUG_PRINTLN();
		}
	}

	for (int i = 0; i < numOfSensors; i++) {
		DEBUG_PRINT(dataToPrint[i].valueName);
		if (i != (numOfSensors - 1))
			DEBUG_PRINT("\t");
		else {
			DEBUG_PRINTLN();
		}
	}

	for (int i = 0; i < numOfSensors; i++) {
		DEBUG_PRINT(dataToPrint[i].value);
		if (i != (numOfSensors - 1))
			DEBUG_PRINT("\t");
		else {
			DEBUG_PRINTLN();
		}
	}
}

/*
void SensorManager::addIDs(measuring* values)
{
	for (int i = 0; i < NUMBER_OF_SENSORS_FOR_APP; i++) {
		values[i].id = i;
	}
}
*/

bool SensorManager::getMeasurementActive()
{
	return measurementActive;
}

void SensorManager::setMeasurementActive(bool active)
{
	measurementActive = active;
}
