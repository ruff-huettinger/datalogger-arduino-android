#include "StateTable.h"

/**
 * @file StateTable.cpp
 *
 * - contains 3 hourTables:
 *     1. sensor only
 *     2. some audio split over the day
 *     3. all-day audio
 * - also contains the BLE hours for the different tables
 *
 *  @author Johannes Brunner
 *
 */

hourTable sensorStateTable[24] = {
							{0, hSensor, 1},
							{1, hSensor, 1},
							{2, hSensor, 1},
							{3, hSensor, 1},
							{4, hSensor, 1},
							{5, hSensor, 1},
							{6, hSensor, 1},
							{7, hSensor, 1},
							{8, hSensor, 1},
							{9, hSensor, 1},
							{10, hBT, 1},
							{11, hSensor, 1},
							{12, hSensor, 1},
							{13, hSensor, 1},
							{14, hSensor, 1},
							{15, hBT, 1},
							{16, hSensor, 1},
							{17, hSensor, 1},
							{18, hSensor, 1},
							{19, hSensor, 1},
							{20, hSensor, 1},
							{21, hSensor, 1},
							{22, hSensor, 1},
							{23, hSensor, 1},
};

hourTable mixedStateTable[24] = {
	{0, hAudio, 1},
	{ 1, hSensor, 1 },
	{ 2, hSensor, 1 },
	{ 3, hSensor, 1 },
	{ 4, hSensor, 1 },
	{ 5, hSensor, 1 },
	{ 6, hAudio, 1 },
	{ 7, hSensor, 1 },
	{ 8, hSensor, 1 },
	{ 9, hSensor, 1 },
	{ 10, hBT, 1 },
	{ 11, hSensor, 1 },
	{ 12, hAudio, 1 },
	{ 13, hSensor, 1 },
	{ 14, hSensor, 1 },
	{ 15, hBT, 1 },
	{ 16, hSensor, 1 },
	{ 17, hSensor, 1 },
	{ 18, hAudio, 1 },
	{ 19, hSensor, 1 },
	{ 20, hSensor, 1 },
	{ 21, hSensor, 1 },
	{ 22, hSensor, 1 },
	{ 23, hSensor, 1 },
};

hourTable audioStateTable[24] = {
	{0, hAudio, 1},
	{ 1, hAudio, 1 },
	{ 2, hAudio, 1 },
	{ 3, hAudio, 1 },
	{ 4, hAudio, 1 },
	{ 5, hAudio, 1 },
	{ 6, hAudio, 1 },
	{ 7, hAudio, 1 },
	{ 8, hAudio, 1 },
	{ 9, hAudio, 1 },
	{ 10, hBT, 1 },
	{ 11, hAudio, 1 },
	{ 12, hAudio, 1 },
	{ 13, hAudio, 1 },
	{ 14, hAudio, 1 },
	{ 15, hBT, 1 },
	{ 16, hAudio, 1 },
	{ 17, hAudio, 1 },
	{ 18, hAudio, 1 },
	{ 19, hAudio, 1 },
	{ 20, hAudio, 1 },
	{ 21, hAudio, 1 },
	{ 22, hAudio, 1 },
	{ 23, hAudio, 1 },
};
