#pragma once

#include <stdint.h>
#include "StateTable.h"
//#include "MemoryFree.h"

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

 // Uncomment for Serial Debugging
#define DEBUG_SERIAL

// Uncomment for using a pure NANO 33 BLE Board without egg
#define DEBUG_BOARD

// Uncomment for mapping hours -> minutes for debugging
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

//#define LOGMEMORY DEBUG_PRINT("Free Memory: "); DEBUG_PRINTLN(freeMemory());

// Settings changed via Android
extern uint32_t startTime;
extern volatile hourTable* activeTable;

// BLE:
const static uint32_t BLE_TIME_MIN = 30;
#define NUMBER_OF_SENSORS_FOR_APP 8


// Sensors:
#define USING_IMU_ACC  // 3x
//#define USING_IMU_GYRO  // 3x
//#define USING_IMU_MAG  // 3x
#define USING_HTS_TEMP  // 1x
#define USING_HTS_HUMI  // 1x
//#define USING_APDS_PROX  // 1x
#define USING_APDS_LIGHT  // 1x
#define USING_ANALOG_LIGHT // 2x
#define USING_BATTERY_SENSOR // 1x


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
} measuring;


// Audio:

// skip first recordings until the mic-electronic is initialized
const static uint32_t AUDIO_FILE_WRITE_DELAY_MS = 1000;

// use a higher GAIN if records are to quiet
// use a lower gain if records overmodulate
const static uint8_t GAIN = 15;

// Keep these settings for 15kHz/8bit Audio
const static uint16_t BUFFER_SIZE = 1024;

const static uint32_t SAMPLE_RATE = 31250;

const static uint8_t BIT_DEPTH = 8;

const static uint8_t NUM_OF_CHANNELS = 1;


/* WAV HEADER STRUCTURE */
struct wavStruct
{
	const char chunkID[4] = { 'R', 'I', 'F', 'F' };
	uint32_t chunkSize = 0; //Size of (entire file in bytes - 8 bytes) or (data size + 36)
	const char format[4] = { 'W', 'A', 'V', 'E' };
	const char subchunkID[4] = { 'f', 'm', 't', ' ' };
	const uint32_t subchunkSize = 16;
	const uint16_t audioFormat = 1;               //PCM == 1
	const uint16_t numChannels = NUM_OF_CHANNELS; //1=Mono, 2=Stereo
	const uint32_t sampleRate = SAMPLE_RATE;
	const uint32_t byteRate = SAMPLE_RATE * NUM_OF_CHANNELS * (BIT_DEPTH / 8); //== SampleRate * NumChannels * BitsPerSample/8
	const uint16_t blockAlign = NUM_OF_CHANNELS * BIT_DEPTH / 8;                                           //== NumChannels * BitsPerSample/8
	const uint16_t bitsPerSample = BIT_DEPTH;                                //8,16,32...
	const char subChunk2ID[4] = { 'd', 'a', 't', 'a' };
	uint32_t subChunk2Size = 0; //== NumSamples * NumChannels * BitsPerSample/8
									 //Data                                       //The audio data
};

/*********************************************************/

