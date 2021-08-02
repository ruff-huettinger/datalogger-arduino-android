/*
  ProximityWakeUpTest

  - A test to wake the Arduino up from System-Off Low-power mode
  - The proximity sensor interrupt line is used as wakeup-callback for the board


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  toDo: Why is "enableInterrupt" not necessary?
  
  Created 14 09 2020
  By Johannes Brunner
  
------------------------------------------------------------------

ProximityInterrupt.ino
APDS-9960 RGB and Gesture Sensor
Shawn Hymel @ SparkFun Electronics
October 24, 2014
https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor

Tests the proximity interrupt abilities of the APDS-9960.
Configures the APDS-9960 over I2C and waits for an external
interrupt based on high or low proximity conditions. Move your
hand near the sensor and watch the LED on pin 13.

Hardware Connections:

IMPORTANT: The APDS-9960 can only accept 3.3V!

 Arduino Pin  APDS-9960 Board  Function

 3.3V         VCC              Power
 GND          GND              Ground
 A4           SDA              I2C Data
 A5           SCL              I2C Clock
 2            INT              Interrupt
 13           -                LED

Resources:
Include Wire.h and SparkFun_APDS-9960.h

Development environment specifics:
Written in Arduino 1.0.5
Tested with SparkFun Arduino Pro Mini 3.3V

This code is beerware; if you see me (or any other SparkFun
employee) at the local, and you've found our code helpful, please
buy us a round!

Distributed as-is; no warranty is given.
*/

#include <Wire.h>
#include "SparkFun_APDS9960.h"

// Pins
#define APDS9960_INT PIN_INT_APDS // Needs to be an interrupt pin
#define LED_PIN 13                // LED for showing interrupt

// Constants
#define PROX_INT_HIGH 50 // Proximity level for interrupt
#define PROX_INT_LOW 0   // No far interrupt

// LOW POWER MODE
#define LOOP_CYCLES 10
#define CYCLE_TIME_MS 500

// Global variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint8_t proximity_data = 0;
int isr_flag = 0;
volatile int cycles = 0;
bool off = false;

void setup()
{

	//toDo: test interrupt wake-up from sleep mode

	delay(1000);

	// Set LED as output
	pinMode(LED_PIN, OUTPUT);
	pinMode(APDS9960_INT, INPUT);

	// Initialize Serial port
	Serial.begin(9600);

	delay(1000);

	Serial.println();
	Serial.println(F("---------------------------------------"));
	Serial.println(F("SparkFun APDS-9960 - ProximityInterrupt"));
	Serial.println(F("---------------------------------------"));

	// Initialize interrupt service routine


	attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);

	//enableWakeupByInterrupt(APDS9960_INT, FALLING);

	//disableWakeupByInterrupt(APDS9960_INT);

	// Initialize APDS-9960 (configure I2C and initial values)
	if (apds.init())
	{
		Serial.println(F("APDS-9960 initialization complete"));
	}
	else
	{
		Serial.println(F("Something went wrong during APDS-9960 init!"));
	}

	// Adjust the Proximity sensor gain
	if (!apds.setProximityGain(PGAIN_2X))
	{
		Serial.println(F("Something went wrong trying to set PGAIN"));
	}

	// Set proximity interrupt thresholds
	if (!apds.setProximityIntLowThreshold(PROX_INT_LOW))
	{
		Serial.println(F("Error writing low threshold"));
	}
	if (!apds.setProximityIntHighThreshold(PROX_INT_HIGH))
	{
		Serial.println(F("Error writing high threshold"));
	}

	// Start running the APDS-9960 proximity sensor (interrupts)
	if (apds.enableProximitySensor(true))
	{
		Serial.println(F("Proximity sensor is now running"));
	}
	else
	{
		Serial.println(F("Something went wrong during sensor init!"));
	}

	//NRF_GPIO->PIN_CNF[APDS9960_INT] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	//NRF_GPIO->PIN_CNF[APDS9960_INT] |= ((uint32_t)GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos);
	//NRF_POWER->SYSTEMOFF = 1;
	//NRF_POWER->TASKS_LOWPWR = 1;

}

void loop()
{

	if (cycles >= LOOP_CYCLES)
	{
		Serial.println("POWER OFF!");
		disableCurrentMonsters();
		NRF_POWER->SYSTEMOFF = 1;
	}

	// If interrupt occurs, print out the proximity level
	else
	{

		if (isr_flag == 1)
		{
			Serial.println("Interupt!");
		}
		Serial.print("Cycles: ");
		Serial.println(cycles);

		// Read proximity level and print it out
		if (!apds.readProximity(proximity_data))
		{
			Serial.println("Error reading proximity value");
		}
		else
		{
			/*
			Serial.print("Proximity detected! Level: ");
			Serial.println(proximity_data);
			*/
		}

		// Turn on LED for a half a second

		if (true)
		{
			digitalWrite(13, HIGH);
			off = false;
		}
		else
		{
			digitalWrite(13, LOW);
			off = true;
		}

		// Reset flag and clear APDS-9960 interrupt (IMPORTANT!)
		isr_flag = 0;
		if (!apds.clearProximityInt())
		{
			Serial.println("Error clearing interrupt");
		}

		cycles++;
		delay(CYCLE_TIME_MS);

	}
}

void interruptRoutine()
{
	cycles = 0;
	isr_flag = 1;
}


/**************************************************************************/
/*!
	@brief enable Sense mode on selected pin
*/
/**************************************************************************/

void enableWakeupByInterrupt(uint32_t pin, uint32_t mode)
{
	NRF_GPIO->PIN_CNF[pin] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	switch (mode)
	{
	case HIGH:
	case RISING:
		NRF_GPIO->PIN_CNF[pin] |= ((uint32_t)GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos);
		break;
	case LOW:
	case FALLING:
		NRF_GPIO->PIN_CNF[pin] |= ((uint32_t)GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
		break;
	default:
		return;
	}
}

/**************************************************************************/
/*!
	@brief disable Sense mode on selected pin
*/
/**************************************************************************/
void disableWakeupByInterrupt(uint32_t pin)
{
	NRF_GPIO->PIN_CNF[pin] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	NRF_GPIO->PIN_CNF[pin] |= ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

void disableCurrentMonsters() {
	digitalWrite(LED_PWR, LOW);
	digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
	digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW);
}
