/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano 33 BLE board

  This example code is in the public domain.
*/

#include <PDMUpdate.h>
#include "sdfat_2_1.h"
#include "SPI.h"


const long buffSize = 8192;
const long numOfBuffs = 30;
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[buffSize];

// number of samples read
volatile int samplesRead;

// File system object.
const int chipSelect = 10;
SdFat SD;
File uFile;

long cnt = 0;

void setup() {
	Serial.begin(9600);
	while (!Serial);

	// configure the data receive callback
	PDM.onReceive(onPDMdata);

	// optionally set the gain, defaults to 20
	// PDM.setGain(30);

	// initialize PDM with:
	// - one channel (mono mode)
	// - a 16 kHz sample rate
	PDM.setBufferSize(buffSize * 2);

	if (!PDM.begin(1, 16000)) {
		Serial.println("Failed to start PDM!");
		while (1);
	}

	PDM.setGain(80);


	if (!SD.begin(chipSelect))
	{
		Serial.println("initialization failed. Things to check:");
		Serial.println("1. is a card inserted?");
		Serial.println("2. is your wiring correct?");
		Serial.println("3. did you change the chipSelect pin to match your shield or module?");
		Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
		while (true)
			;
	}

	uFile = SD.open("testii2", FILE_WRITE);
}

void loop() {
	// wait for samples to be read
	if (samplesRead) {
		//uFile = SD.open("testii2", FILE_WRITE);
		// print samples to the serial monitor or plotter
		for (int i = 0; i < samplesRead; i++) {
			uFile.write((lowByte(sampleBuffer[i])));
			uFile.write((highByte(sampleBuffer[i])));
		}
		cnt++;

		//uFile.close();
		// clear the read count
		samplesRead = 0;
		while (samplesRead == 0) {};
	}

	/*
	else {
		//uFile = SD.open("testii", FILE_WRITE);
		uFile.write();
	}
	*/

	if (cnt == numOfBuffs) {
		digitalWrite(LED_PWR, LOW);
		uFile.close();
		while (true);
	}
}

void onPDMdata() {
	// query the number of bytes available
	int bytesAvailable = PDM.available();

	// read into the sample buffer
	PDM.read(sampleBuffer, bytesAvailable);

	// 16-bit, 2 bytes per sample
	samplesRead = bytesAvailable / 2;
}