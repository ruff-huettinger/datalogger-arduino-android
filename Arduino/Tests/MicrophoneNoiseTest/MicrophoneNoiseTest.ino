/*
  MicrophoneNoiseTest

  - A test to record 16bit-Audio with higher sampling frequency buffered on a sd-card


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Micro-SD card connected via SPI and CS on pin 10

  Created 05 10 2021
  By Johannes Brunner

*/

#include "PDMUpdate.h"
#include <SPI.h>
#include "SdFat.h"


const uint32_t MY_BUFFER_SIZE = 98304;

const uint16_t PDM_BUFFER_SIZE = 1024;

const uint16_t NUM_OF_MY_BUFFERS = 3;

// use 16000 31250 41667 -> best results with 31250
const uint16_t SAMPLE_RATE = 16000;

const uint8_t BIT_DEPTH = 16;

const uint8_t NUM_OF_CHANNELS = 1;

const uint8_t GAIN = 10;

char* filename = "myWav_16k_16bitDUP.wav"; //The name of the file that will be generated


// buffer to read samples into, each sample is 16-bits
short myBuffer[MY_BUFFER_SIZE];
uint32_t bufferOffset = 0;
uint32_t myLastSaveTime = 0;
volatile bool myISRFlag = false;
volatile bool pdmFlag = false;
int bytesAvailable = 0;
int cnt = 0;

// File system object.
const int chipSelect = 10;
SdFat SD;
File uFile;

void setup()
{
	Serial.begin(115200);
	while (!Serial)
		;

	// configure the data receive callback
	PDM.onReceive(onPDMdata);

	// optionally set the gain, defaults to 20
	PDM.setGain(GAIN);

	// set Buffer to double size of samples buffer for lib's double buffering
	//PDM.setBufferSize(BUFFER_SIZE * 2);

	if (!PDM.begin(NUM_OF_CHANNELS, SAMPLE_RATE))
	{
		Serial.println("Failed to start PDM!");
		while (1)
			;
	}

	Serial.print("Initializing SD card...");

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

	Serial.println("initialization done.");

	uFile = SD.open(filename, FILE_WRITE);

	if (!uFile)
	{
		Serial.print("Failed to open file");
		while (true)
			;
		return;
	}
}

void loop()
{
	if (myISRFlag && cnt < NUM_OF_MY_BUFFERS) {
		Serial.println("buffer is full");
		saveMyBufferOnSD();
		bufferOffset = 0;
		myLastSaveTime = millis();
		myISRFlag = false;
		cnt++;
	}

	if (cnt >= NUM_OF_MY_BUFFERS) {
		uFile.close();
		Serial.print("finished");
		while (true);
	}
}

void saveMyBufferOnSD() {
	for (int i = 0; i < MY_BUFFER_SIZE; i++)
	{
		uFile.write(myBuffer[i]);
	}
	Serial.println("buffer was saved");
}

void onPDMdata()
{
	// query the number of bytes available
	bytesAvailable = PDM.available();
	int throwAwayBuffer[PDM_BUFFER_SIZE];

	if (millis() > (myLastSaveTime + 2000)) {
		if ((bufferOffset + bytesAvailable) < MY_BUFFER_SIZE) {
			// read into the my buffer
			PDM.read(&myBuffer[bufferOffset], bytesAvailable);
		}
		else {
			myISRFlag = true;
			PDM.read(throwAwayBuffer, (bytesAvailable));
		}
		bufferOffset += (bytesAvailable / 2);
	}
	else {
		PDM.read(throwAwayBuffer, bytesAvailable);
	}
}
