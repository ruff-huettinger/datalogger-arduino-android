#pragma once

#include "MemoryFree.h"
#include <stdint.h>
#include "StateTable.h"

/**
 * @file config.h
 *
 * - holds the sensor-definition for measurings
 * - contains project-scope variables changable by BLE-device
 * - handles debug modes and error feedback
 * - includes constants
 *
 * @author Johannes Brunner
 *
 */

 // Uncomment for fast compiling with Visual Micro
#define FAST_COMPILE

 // Uncomment for using the new SHTC3-Temp sensor
 //#define USING_SHTC3

// Uncomment for erasing and formatting the sd card when initializing
#define FORMAT_SD

 // Uncomment for Serial Debugging
//#define DEBUG_SERIAL

// Uncomment for using a pure NANO 33 BLE Board without egg
//#define DEBUG_BOARD

 // Uncomment for adding memory information to sensor-data
#define DEBUG_MEMORY

// Uncomment for setting a manual hour mode
//#define DEBUG_TIME

// Uncomment for setting a manual start time instead of using BLE phone time
//#define MANUAL_START_TIME

#ifdef DEBUG_SERIAL
#define DEBUG_BEGIN(x) Serial.begin(x)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_BEGIN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#define LOGMEMORY DEBUG_PRINT("Free Memory: "); DEBUG_PRINTLN(freeMemory());

// Settings changed via Android
extern uint32_t startTime;
extern volatile hourTable* activeTable;

// BLE:
const static uint32_t BLE_TIME_MIN = 59;
const static uint32_t BLE_TIME_SEC = 59;
#define NUMBER_OF_SENSORS_FOR_APP 11


// Sensors:
#define USING_IMU_ACC  // 3x
//#define USING_IMU_GYRO  // 3x
#define USING_IMU_MAG  // 3x
#define USING_HTS_TEMP  // 1x
#define USING_HTS_HUMI  // 1x
//#define USING_APDS_PROX  // 1x
#define USING_APDS_LIGHT  // 1x
#define USING_ANALOG_LIGHT // 2x
#define USING_BATTERY_SENSOR // 1x
//#define USING_HTS_INTERN  // 2x


enum SENSORID
{
	ACCX,
	ACCY,
	ACCZ,
	MAGX,
	MAGY,
	MAGZ,
	TEMP,
	HUMI,
	LIGHT,
	LIGHTANAONE,
	LIGHTANATWO,
	BATSENS,
	INTTEMP,
	INTHUMI,
};


// Pins:
const static uint8_t PIN_ENABLE_ANALOG_LIGHT = 5;
const static uint8_t PIN_READ_ANALOG_LIGHT_1 = 16;
const static uint8_t PIN_READ_ANALOG_LIGHT_2 = 15;
const static uint8_t PIN_ENABLE_SD_MODUL = 3;
const static uint8_t PIN_ENABLE_HTS = 4;
const static uint8_t ENABLE_BATTERY_MEASURE_PIN = 6;
const static uint8_t READ_BATTERY_PIN = 21;

typedef struct measuring {
	char* sensorName;
	char* valueName;
	float value;
	uint8_t id;
} measuring;


// Audio:

// skip first recordings until the mic-electronic is initialized
const static uint32_t AUDIO_FILE_WRITE_DELAY_MS = 2000;

// use a higher GAIN if records are to quiet
// use a lower gain if records overmodulate
const static uint8_t GAIN = 80;

// keep these settings for best-quality 15kHz/8bit Audio
const static uint16_t BUFFER_SIZE = 256;

const static uint32_t SAMPLE_RATE = 31250;

const static uint8_t BIT_DEPTH = 16;

const static uint8_t NUM_OF_CHANNELS = 1;


/* WAV HEADER STRUCTURE */
struct wavStruct
{
	const char chunkID[4] = { 'R', 'I', 'F', 'F' };
	uint32_t chunkSize = 0; // change after recording
	const char format[4] = { 'W', 'A', 'V', 'E' };
	const char subchunkID[4] = { 'f', 'm', 't', ' ' };
	const uint32_t subchunkSize = 16;
	const uint16_t audioFormat = 1;               //PCM == 1
	const uint16_t numChannels = NUM_OF_CHANNELS; //1=Mono, 2=Stereo
	const uint32_t sampleRate = SAMPLE_RATE;
	const uint32_t byteRate = SAMPLE_RATE * NUM_OF_CHANNELS * (BIT_DEPTH / 8);
	const uint16_t blockAlign = NUM_OF_CHANNELS * BIT_DEPTH / 8;
	const uint16_t bitsPerSample = BIT_DEPTH;
	const char subChunk2ID[4] = { 'd', 'a', 't', 'a' };
	uint32_t subChunk2Size = 0; // change after recording
};

/*********************************************************/

