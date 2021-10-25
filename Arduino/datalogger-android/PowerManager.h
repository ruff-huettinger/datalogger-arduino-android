#pragma once

#include <Arduino.h>
#include "config.h"

/**
 * @file PowerManager.h
 *
 * - contains functions for reaching low power
 * - also acts as the battery sensor
 *
 * @author Johannes Brunner
 *
 */

class PowerManager
{
private:
	uint32_t NUM_OF_BATTERY_READINGS = 100;

	uint16_t MAX_ANALOG_READ = 656; //@ 4.1 V input
	uint16_t MIN_ANALOG_READ = 513; //@ 3.3 V input

	void initExternHTS();

	void initSDModul();

	void initAnalogLights();

	void initBattery();

	float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);

public:
	void init();
	uint16_t getAnalogAvg();

	float getBatteryFraction();

	void disableCurrentConsumers();
	void enableOnBoardSensors();

	void enableHFCLK();
	void disableHFCLK();

	void enableLFCLK();
	void disableLFCLK();
	void swapLFCLKSource();

	void resetLFCLK();

	PowerManager() {};
	~PowerManager() {};
};

