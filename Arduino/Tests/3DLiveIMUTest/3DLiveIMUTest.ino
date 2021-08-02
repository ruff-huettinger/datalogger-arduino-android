/*
  3DLiveIMUTest

  - Live tracking of eggs position using the on-board IMU 
  - A complex madgwick filter using quaternions is applied (https://github.com/kriswiner/LSM9DS1/issues/14)
  - The calculated euler-angles (yaw, pitch and roll) are sent to the corresponding Android-App created with unity
  - The sparkfun-lib (SensorMotion.h) could not achieve as good results as the Madgwick filter (Accelerometer.h) 

  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Corresponding Android-App: "Electronic egg live tracking"

  Created 17 05 2021
  By Johannes Brunner

*/

#include "Accelerometer.h"
#include <ArduinoBLE.h>
#include "BLEManager.h"

#define FAST_COMPILE

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
#include "MemoryFree.h"
#endif // FAST_COMPILE



// Variables:

BLEManager bm;
Acceleromter acc;
measuring  x[1] = { "0", "0", 0.0f };

// toDo: apply the changes to the electronic egg operation

void setup() {
	delay(1000);

	bm.init();

	acc.setup();

	bm.begin(x, 1, 0.1);

	delay(1000);
}

uint32_t timeTilt = 0;
void loop() {
	
	acc.loop();

	bm.run();

	if (millis() > timeTilt + 50) {
		bm.refreshTilt();
		timeTilt = millis();
	}
}

float getRndSensorValue() {
	return random(1, 5000) / 100.0;
}
