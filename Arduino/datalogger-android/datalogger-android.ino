/*
	Electronic Egg-Operation Version 2 for unity app (multi-threaded)

	The circuit:
	* Board: Arduino Nano 33 BLE Sense
	* Adafruit micro-sd shield with card inserted
	* External battery voltage supply and connected on D6 + D7


	Created 20 05 2021
	By Johannes Brunner


*/

// Uncomment for fast compiling with Visual Micro
#define FAST_COMPILE

#include <mbed.h>
#include "BLEManager.h"
#include "FileManager.h"
#include "SensorManager.h"
#include "Recorder.h"
#include "config.h"
#include "StateTable.h"
#include "TimeManager.h"

#ifdef FAST_COMPILE
// Libs included here for visual micro compiler
#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <Wire.h>
#include "ArduinoBLE.h"
#include "SdFat.h"
#include "ArduinoPDM.h"
#include "Arduino_APDS9960.h"
#include "SparkFunLSM9DS1.h"
#include "Arduino_HTS221.h"
#include "HTS221.h"
#include "SparkFun_SHTC3.h"
#include "MemoryFree.h"
#endif // FAST_COMPILE

using namespace rtos;
using namespace mbed;

// Variables:

// ----Project-scope definitions from config.h--------

// Unix-time of initialization, set by BLE-peripheral
uint32_t startTime = 0;
// Definition for active table/mode
volatile hourTable* activeTable = sensorStateTable;

//--------------------------------------------------

// Thread for BLE-connection
Thread* btThread = NULL;


// Objects
BLEManager bm;
FileManager fm;
SensorManager sm;
Recorder r;
TimeManager tman;
PowerManager pm;

// Holds last sensor measurements
measuring* sensorData = NULL;
// Number of active sensors
uint8_t sensorDataLength = 0;

const char projectName[] = "S-10926-Elektronisches Ei";
const char ver[] = "Prototyp:2.0 with UnityApp";


// toDo: write documentation for changes to ble
// toDo: change audio-recorder file lengths -> independent from sensor-interval


// override the default main function to remove USB CDC feature
int main(void)
{
	init();
	initVariant();

#ifdef DEBUG_SERIAL
	//remove USB CDC feature
#if defined(SERIAL_CDC)
	PluggableUSBD().begin();
	SerialUSB.begin(115200);
#endif
#endif // DEBUG_SERIAL
	setup();
	while (true) {
		loop();
	}
	return 0;
}

void setup() {

#ifdef DEBUG_SERIAL
	DEBUG_BEGIN(115200);
	// wait 5 secs for optional serial connection
	while (!Serial && millis() < 5000);
	delay(100);
	DEBUG_PRINTLN(projectName);
	DEBUG_PRINTLN(ver);
	delay(100);
#endif // DEBUG_SERIAL

	// disable UART to save energy
	pm.disableCurrentConsumers();

	// initialize extern hardware
	pm.init();

	delay(100);

	// turn off power led
	digitalWrite(LED_PWR, LOW);

	// initialize SD-Card and create a new folder for this run

	fm.init();

	// initialize the audio-recorder callback
	r.init();

	// initialize BLE services
	bm.init();

	sensorDataLength = sm.getDataLength();
	sensorData = new measuring[sensorDataLength];


	// set starting time
#ifdef MANUAL_START_TIME
	startTime = 1613778600; // Set RTC time to 11:50 for debuging
	set_time(startTime);
#else

	if (!initializedByBLEUser()) {
		// wait for a BLE-user to set time and recording settings and to start
	}
#endif // !MANUAL_START_TIME

#ifdef FORMAT_SD
	fm.eraseCard();
	fm.formatCard();
#endif // FORMAT_SD

	fm.createFolder();
	fm.clearSPI();

	DEBUG_PRINTLN("Initialization done");
}


void loop() {

	//LOGMEMORY; //-> uncomment for debugging Memory leaks

	//---- 1. Measure every sensor and write line to sd-card .csv-file

	recordSensors();

	//sm.printSensorValues(sensorData, sensorDataLength); //-> uncomment for printing the sensors

//---- 2. Get current Time, Timedif to next measurement and hours-mode from ActiveTable (set via BLE)
	double sensorInterval = tman.getDiffTime() * 1000;
	tm* currentTime = tman.getCurrentTime();
#ifdef DEBUG_TIME
	modeOfTheHour currentMode = activeTable[0].hourMode;
#else 
	modeOfTheHour currentMode = activeTable[currentTime->tm_hour].hourMode;
	DEBUG_PRINTLN();
#endif // DEBUG_TIME

#ifdef DEBUG_SERIAL
	char printbuf[32];
	tman.getTimeStampTime(printbuf);
	DEBUG_PRINT(printbuf);
	DEBUG_PRINT(": ");
	DEBUG_PRINTLN("Sensor measuring complete");
#endif // DEBUG_SERIAL


	/*---- 3. Decide how to continue until next measurement depending on hourmode:
			  - Sleep
			  - Record audio
			  - Start BT-Thread + Sleep
	*/
	if (currentMode == hSensor) {
		sleepWithLowPower(sensorInterval);
	}

	else if (currentMode == hAudio) {
		// Don't run BLE and audio at the same time!
		if (!bm.isRunning()) {
			char timestamp[32];
			DEBUG_PRINTLN("Audio started");
			tman.getTimeStampDate(timestamp);
			char filename[32];
			sprintf(filename, "%s.wav", timestamp);
			recordAudio(filename, sensorInterval);
			finishRecording(filename);
			DEBUG_PRINTLN("Audio finished");
		}
		else {
			sleepWithLowPower(sensorInterval);
		}
	}

	else if (currentMode == hBT) {
	#ifdef DEBUG_TIME
		if (!bm.isRunning())
		#else
		if (currentTime->tm_min >= 0 && currentTime->tm_min < (BLE_TIME_MIN) && !bm.isRunning())
		#endif // DEBUG_TIME
		{
			DEBUG_PRINTLN("Starting BT Thread");
			delete btThread;
			btThread = new Thread();
			btThread->start(threadFunctionBLE);
	}
		sleepWithLowPower(sensorInterval);
}
}

bool initializedByBLEUser() {

	// get sensor values and battery one time for displaying it in the app 
	sm.getSensorValues(sensorData);

	bm.begin(sensorData, sensorDataLength, pm.getBatteryFraction(), fm.getCardSize(), fm.getWrittenBytes());

	// time is set automatically on BLE device connect
	while (!bm.isTimeSet()) {
		bm.run();
	}
	startTime = bm.getStartTime();

	set_time(startTime);

	// program starts when user presses start button in app
	while (!bm.isStarted()) {
		bm.run();
	}

	bm.end();

	pm.resetLFCLK();
	pm.disableHFCLK();

	setBLEUpdates();

	return true;
}

// Thread-function for bluetooth
void threadFunctionBLE() {
	DEBUG_PRINT("ble started");

	pm.enableHFCLK();

	bm.begin(sensorData, sensorDataLength, pm.getBatteryFraction(), fm.getCardSize(), fm.getWrittenBytes());

	tm* time = tman.getCurrentTime();


	// only run BLE from XX:00 to XX:BLE_TIME_MIN (eg 09:00 to 09:30)
	while ((time->tm_min < BLE_TIME_MIN) || (time->tm_sec < BLE_TIME_SEC))

	{
		bm.run();
		time = tman.getCurrentTime();
	}

	// Close BT Connection
	bm.setConnectionState(false);

	bm.end();
	pm.resetLFCLK();

	if (!r.isRecording()) {
		pm.disableHFCLK();
	}

	setBLEUpdates();

	DEBUG_PRINTLN("Terminate BT Thread");
	btThread->terminate();
}

void recordSensors() {
	// fill data with values from sensor manager

	while (sm.getMeasurementActive()) {
		//DEBUG_PRINTLN("measurement blocked");
		//return;
	}

	sm.setMeasurementActive(true);
	sm.getSensorValues(sensorData);
	sm.setMeasurementActive(false);

	char timestamp[32];
	tman.getDateTimeFormatted(timestamp);

	fm.openFile();

#ifdef DEBUG_MEMORY
	tm* currentTime = tman.getCurrentTime();
	int currentMode = (int)activeTable[currentTime->tm_hour].hourMode;
#else
	int currentMode = -1;
#endif // DEBUG_MEMORY
	fm.writeSensorsLine(sensorData, sensorDataLength, timestamp, currentMode);
	fm.closeFile();
}

void recordAudio(char* filename, double timedif) {
	fm.createAudioFile(filename);
	fm.openFile(filename);
	r.begin();
	fm.writeWavHeader(timedif);
	DEBUG_PRINTLN("Start recording");
	uint32_t writingAudioTime = millis();
	uint32_t timeBefStart = millis();

	while ((millis() - timeBefStart) < (int)timedif) {
		while (!r.isBufferFull()) {
			//idle
		}
	#ifdef DEBUG_SERIAL
		if (millis() - writingAudioTime < 1) {
			//DEBUG_PRINTLN("SD-card too slow for audio!");
		}
	#endif // DEBUG_SERIAL
		int16_t* sampleBuffer = r.getRecordedSamples();
		// convert to 8-bit signed audio 
		for (int i = 0; i < BUFFER_SIZE; i++) {
			constrain(sampleBuffer[i], -127, 127);
			// transform from int8 to uint8
			sampleBuffer[i] += 128;
		}
		if ((millis() - timeBefStart) > AUDIO_FILE_WRITE_DELAY_MS) {
			fm.writeAudioData(sampleBuffer);
		}
		r.setBufferEmpty();
		writingAudioTime = millis();
	}
}

void finishRecording(char* filename) {
	r.stop();
	if (!bm.isRunning()) {
		pm.disableHFCLK();
	}
	fm.finalizeWav();
	fm.closeFile(filename);
}

void sleepWithLowPower(uint32_t sleepInterval) {
	DEBUG_PRINT("Sleep started for (s): ");
	DEBUG_PRINTLN(sleepInterval / 1000);
	thread_sleep_for(sleepInterval);
	DEBUG_PRINTLN("Sleep finished");
}

void setBLEUpdates() {
	byte newInterval = bm.getUpdatedInterval();
	byte* x = bm.getUpdatedTable();

	for (int i = 0; i < 24; i++) {
		activeTable[i].hourMode = (modeOfTheHour)x[i];
		activeTable[i].numOfMeasures = newInterval;
	}
}

