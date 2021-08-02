/*
	Electronic Egg-Operation (multi-threaded)

	The circuit:
	* Board: Arduino Nano 33 BLE Sense
	* Adafruit micro-sd shield with card inserted
	* External battery voltage supply and connected on D6 + D7


	Created 09 10 2020
	By Johannes Brunner


*/

#define FAST_COMPILE

#include <mbed.h>
/*
#include "BLEManager.h"
#include "FileManager.h"
#include "SensorManager.h"
#include "Recorder.h"
#include "config.h"
#include "StateTable.h"
*/

#ifdef FAST_COMPILE
// Libs included here for visual micro compiler
#include <Arduino.h>
#include "ArduinoBLE.h"
#include <stdint.h>
#include <SPI.h>
#include "SdFat.h"
#include "PDMDup.h"
#include "Arduino_APDS9960.h"
#include <Wire.h>
#include "SparkFunLSM9DS1.h" // sparkfun version
#include "Arduino_HTS221.h"
#include "HTS221.h"
#include "MemoryFree.h"
#endif // FAST_COPMPILE

using namespace rtos;
using namespace mbed;

// Variables:
//Thread tSensors;
//Thread tBluetooth;
//BLEManager bm;
//FileManager fm;
//SensorManager sm;
//Recorder r;

//stateHolder* table = sensorStateTable;

uint16_t sensorInterval;
uint16_t bleInterval;
uint32_t startTime;

// toDo: use timing for different states 
// toDo: measure power consumption

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

	//fm.init();
	//fm.createFolder();

	//r.init();

	/*
	bm.init();
	while (!bm.isInitialized()) {
		bm.run();
	}*/

	//startTime = 1610619981;

	//set_time(startTime);  // Set RTC time to Wed, 28 Oct 2009 11:35:37
	//set_tzo_min(60);

	setup();
	return 0;
}

void setup() {
	digitalWrite(LED_PWR, LOW);
	disableCurrentConsumers();

	/*

	sensorInterval = 1000;
	bleInterval = 5000;
	//digitalWrite(LED_PWR, HIGH);

#ifdef DEBUG_SERIAL
	Serial.begin(115200);
	// wait for optional serial connection
	while (!Serial && millis() < 3000);
#endif // DEBUG_SERIAL

	// Start BLE

	// Timing

	time_t seconds = time(NULL);
	tm* time = localtime(&seconds);
	char buffer[32];
	strftime(buffer, 32, "%I:%M %p\n", localtime(&seconds));
	Serial.printf("Time as a custom formatted string = %s", buffer);

	uint8_t currentHour = time->tm_hour;
	uint8_t currentMinute = time->tm_min;
	Serial.println(currentHour);
	Serial.println(currentMinute);


	states currentState = table[currentHour].state;

	uint32_t nextStateChangeTime = nextStateChange(currentMinute, table[currentHour].numOfMeasures);
	Serial.println(nextStateChangeTime);

	//tBluetooth.start(tBluetoothF);
	//tSensors.start(tSensorsF);
	Serial.println("Setup finished");
	//ThisThread::sleep_for(10000);

	*/
	while (true) {
		delay(10000);
	}
}

uint32_t nextStateChange(uint8_t currentMinute, uint8_t measuresPerHour) {
	uint8_t* times = new uint8_t[measuresPerHour];
	for (int i = 0; i < measuresPerHour; i++) {
		times[i] = i * (60 / measuresPerHour);
		if (times[i] > currentMinute) {
			uint8_t val = times[i];
			delete[] times;
			return val;
		}
	}
	return 0;
	delete[] times;
}

/*

// Thread-function for bluetooth
void tBluetoothF() {
	while (true) {
		LOGMEMORY;
		bm.init();
		while (true) {
			bm.run();
			if (bm.disconnected_) {
				bm.disconnected_ = false;
				bm.end();
				break;
			}
		}
		Serial.println("sleeping ble");
		thread_sleep_for(bleInterval);
		Serial.println("waking ble");
	}
}

// Thread-function for sensors and audio
void tSensorsF() {
	while (true) {
		recordSensors();
		recordAudio();
		LOGMEMORY;
		Serial.println("sleeping sensors");
		thread_sleep_for(sensorInterval);
		Serial.println("waking sensors");
	}
}

void recordSensors() {
	uint8_t dataLength = sm.getDataLength();

	// allocate the array
	measuring* data = new measuring[dataLength];
	// don't forget to delete[] data

	//initialize whole array with default error messages
	for (int i = 0; i < dataLength; i++) {
		data[i].sensorName = "defSensor";
		data[i].valueName = "defValueName";
		data[i].value = 1234.0f;
	}

	// fill data with values from sensor manager
	sm.getSensorValues(data);

	// toDo: handle Time
	//refreshTimeStamp();

	time_t seconds = time(NULL);
	//char* timeStamp_ = ctime(&seconds);

	char buffer[32];
	//strftime(buffer, 32, "%I:%M %p\n", localtime(&seconds));
	strftime(buffer, 32, "%x %X", localtime(&seconds));
	char* timeStamp_ = buffer;

	fm.openFile();
	fm.writeSensorsLine(data, dataLength, timeStamp_);
	fm.closeFile();

	delete[] data;
}

void recordAudio() {
	time_t seconds = time(NULL);
	char timeStamp[20];
	sprintf(timeStamp, "%ld.wav", seconds);
	fm.createAudioFile(timeStamp);
	fm.openFile(timeStamp);
	r.begin();
	fm.writeWavHeader();
	Serial.println("Start recording");
	uint32_t audioTime = millis();

	for (int i = 0; i < NUM_OF_BUFFERS; i++) {
		while (!r.bufferFilled) {
			//idle
		}
	#ifdef DEBUG_SERIAL
		if (i != 0 && millis() - audioTime < 3) {
			//Serial.println("SD-card too slow for audio!");
		}
	#endif // DEBUG_SERIAL
		int16_t* sampleBuffer = r.getRecordedSamples();
		// convert to 8-bit signed audio
		for (int i = 0; i < BUFFER_SIZE; i++) {
			constrain(sampleBuffer[i], -127, 127);
			// transform from int8 to uint8
			sampleBuffer[i] += 128;
		}
		fm.writeAudioData(sampleBuffer);
		r.bufferFilled = 0;
		audioTime = millis();
	}
	r.stop();
	fm.finalizeWav();
	fm.closeFile(timeStamp);
	Serial.println("finished recording");
}

void logTime() {
	time_t seconds = time(NULL);

	Serial.printf("Time as seconds since January 1, 1970 = %u\n", (unsigned int)seconds);

	Serial.printf("Time as a basic string = %s", ctime(&seconds));

	char buffer[32];
	strftime(buffer, 32, "%I:%M %p\n", localtime(&seconds));
	Serial.printf("Time as a custom formatted string = %s", buffer);
}

*/

void disableCurrentConsumers() {
	digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
	digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW);

	//Disabling UART0 (saves around 300-500mA) - @Jul10199555 contribution

	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_STOPRX = 1;
	NRF_UART0->ENABLE = 0;


	*(volatile uint32_t*)0x40002FFC = 0;
	*(volatile uint32_t*)0x40002FFC;
	*(volatile uint32_t*)0x40002FFC = 1; //Setting up UART registers again due to a library issue
}
