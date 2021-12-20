#pragma once

#include <stdint.h>

/**
 * @file StateTable.h
 *
 * - holds the definitions for hour-modes and hour-tables
 * - contains declarations of 3 pre-defined tables
 *
 * @author Johannes Brunner
 *
 */

enum modeOfTheHour {
	hSensor,
	hAudio,
	hBT,
};

typedef struct hourTable {
	uint8_t hour;
	modeOfTheHour hourMode;
	uint8_t numOfMeasures; 
} hourTable;

extern hourTable sensorStateTable[24];

extern hourTable audioStateTable[24];

extern hourTable mixedStateTable[24];

