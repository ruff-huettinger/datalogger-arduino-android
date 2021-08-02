/*
  Take humidity and temperature readings with the SHTC3 using I2C
  By: Owen Lyke
  SparkFun Electronics
  Date: August 24 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Example1_BasicReadings
  To connect the sensor to an Arduino:
  This library supports the sensor using the I2C protocol
  On Qwiic enabled boards simply connnect the sensor with a Qwiic cable and it is set to go
  On non-qwiic boards you will need to connect 4 wires between the sensor and the host board
  (Arduino pin) = (Display pin)
  SCL = SCL on display carrier
  SDA = SDA
  GND = GND
  3.3V = 3.3V
*/

#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
#include "Wire.h"
#include "MemoryFree.h"

SHTC3 mySHTC3;              // Declare an instance of the SHTC3 class
SHTC3_Status_TypeDef result;           // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object


void setup() {
	Serial.begin(115200);                                  // Begin Serial 
	while (Serial == false) {};                              // Wait for the serial connection to start up                                           // Give time to read the welcome message and device ID. 
}

void loop() {
	Serial.print(freeMemory());
	Serial.print(": ");
	begin();
	read();
	end();
	delay(20);
}

void begin() {
	Wire.begin();
	Serial.print("Begin");           // Most SHTC3 functions return a variable of the type "SHTC3_Status_TypeDef" to indicate the status of their execution 
	errorDecoder(mySHTC3.begin());
};

void read() {
	result = mySHTC3.update();             // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object
	printInfo();
}

void end() {
	errorDecoder(mySHTC3.sleep());
	Wire.end();
}



///////////////////////
// Utility Functions //
///////////////////////
void printInfo()
{
	if (mySHTC3.lastStatus == SHTC3_Status_Nominal)              // You can also assess the status of the last command by checking the ".lastStatus" member of the object
	{
		Serial.print("RH = ");
		Serial.print(mySHTC3.toPercent());                        // "toPercent" returns the percent humidity as a floating point number
		Serial.print("%, T = ");
		Serial.print(mySHTC3.toDegC());                           // "toDegF" and "toDegC" return the temperature as a flaoting point number in deg F and deg C respectively 
		Serial.println(" deg C");
	}
	else
	{
		Serial.print("Update failed, error: ");
		errorDecoder(mySHTC3.lastStatus);
		Serial.println();
	}
}

void errorDecoder(SHTC3_Status_TypeDef message)                             // The errorDecoder function prints "SHTC3_Status_TypeDef" resultsin a human-friendly way
{
	switch (message)
	{
	case SHTC3_Status_Nominal: Serial.print("Nominal"); break;
	case SHTC3_Status_Error: Serial.print("Error"); break;
	case SHTC3_Status_CRC_Fail: Serial.print("CRC Fail"); break;
	default: Serial.print("Unknown return code"); break;
	}
}