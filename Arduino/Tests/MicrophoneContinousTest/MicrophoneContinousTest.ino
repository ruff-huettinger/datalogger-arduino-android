/*
  MicrophoneContinousTest

  - A test to record continously to sd card choosing different settings

  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Micro-SD card connected via SPI and CS on pin 10

  Created 21 10 2021
  By Johannes Brunner

*/

#include "PDMUpdate.h"
#include <SPI.h>
#include "SdFat_2_1.h"

#define DISABLE_PRINT 0

#ifndef DISABLE_PRINT
// disable Serial output
#define Serial SomeOtherwiseUnusedName
static class {
public:
	void begin(...) {}
	void print(...) {}
	void println(...) {}
} Serial;
#endif

const uint32_t MY_BUFFER_SIZE = 8192;

const uint32_t NUM_OF_RUNS = 10;

// use 16000 31250 41667 -> best results with 31250
const uint16_t SAMPLE_RATE = 41667;

const uint8_t BIT_DEPTH = 16;

const uint8_t NUM_OF_CHANNELS = 1;

const uint8_t GAIN = 80;

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
	uint32_t byteRate = SAMPLE_RATE * NUM_OF_CHANNELS * (BIT_DEPTH / 8); //== SampleRate * NumChannels * BitsPerSample/8
	uint16_t blockAlign = NUM_OF_CHANNELS * BIT_DEPTH / 8;                                           //== NumChannels * BitsPerSample/8
	uint16_t bitsPerSample = BIT_DEPTH;                                //8,16,32...
	const char subChunk2ID[4] = { 'd', 'a', 't', 'a' };
	uint32_t subChunk2Size = 0; //== NumSamples * NumChannels * BitsPerSample/8
};

short myBuffer[MY_BUFFER_SIZE];
volatile bool bufferFullFlag = false;
int bytesAvailable = 0;
int numOfRunsCount = 0;

// File system object.
const int chipSelect = 10;
SdFat SD;
File uFile;

// just for logging times
long lastBufferFullTime = 0;
long lastBufferSaveTime = 0;


// toDo: test repeating SD-card noise with egg
// toDo: adjust gain of egg

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
	PDM.setBufferSize(MY_BUFFER_SIZE * 2);

	if (!PDM.begin(NUM_OF_CHANNELS, SAMPLE_RATE))
	{
		Serial.println("Failed to start PDM!");
		while (1)
			;
	}

	PDM.setGain(GAIN);

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

	selectFolder();

	uFile = SD.open("realtimetest.wav", FILE_WRITE);
}

void loop()
{
	if (bufferFullFlag) {
		Serial.print("Buffer full @: ");
		Serial.println(lastBufferFullTime);
		bufferFullFlag = false;
		saveMyBufferOnSD(16, 0);
		//printMyBuffer();

		Serial.print("Buffer saved @: ");
		Serial.println(lastBufferSaveTime);
		numOfRunsCount++;
	}

	if (numOfRunsCount == NUM_OF_RUNS) {
		finalizeWav();
		uFile.close();
		digitalWrite(LED_PWR, LOW);
		Serial.print("finished");
		while (true);
	}
}


void saveMyBufferOnSD(int bitdeph, int noiseGate) {

	if (numOfRunsCount == 0) {
		writeWavHeader(bitdeph);
	}

	if (bitdeph == 16) {
		for (int i = 0; i < MY_BUFFER_SIZE; i++)
		{
			uFile.write((lowByte(myBuffer[i])));
			uFile.write((highByte(myBuffer[i])));
			//Serial.print(i);
			//Serial.print(": ");
			//Serial.println(myBuffer[i]);
		}
	}
	else if (bitdeph == 8) {
		for (int i = 0; i < MY_BUFFER_SIZE; i++) {

			uint8_t data = map(myBuffer[i], -32768, +32767, 0, 255);

			if (128 - noiseGate <= data && data <= 128 + noiseGate)
				data = 128;

			uFile.write(data);
		}
	}

	lastBufferSaveTime = micros();
}

void printMyBuffer() {
	for (int i = 0; i < MY_BUFFER_SIZE; i++) {
		Serial.print(i);
		Serial.print(": ");
		Serial.println(myBuffer[i]);
	}
}


void onPDMdata()
{
	// query the number of bytes available
	bytesAvailable = PDM.available();

	//PDM.read(&myBuffer[0], bytesAvailable);
	PDM.read(myBuffer, bytesAvailable);

	bufferFullFlag = true;

	lastBufferFullTime = micros();
}

void selectFolder()
{
	char folderChar_[16];
	int folderNumber_ = findFolderNumber();
	sprintf(folderChar_, "%d", folderNumber_);

	const char* dir = folderChar_;
	// Create a new folder.
	if (!SD.exists(dir)) {
		if (!SD.mkdir(dir)) {
		}
	}

	SD.chdir(dir);

	Serial.print("Selected folder: ");
	Serial.println(folderChar_);
}

int findFolderNumber()
{
	char buffer[16];
	int a = 0;
	sprintf(buffer, "%d", a);
	const char* x = buffer;
	while (SD.exists(x)) {
		a++;
		sprintf(buffer, "%d", a);
		x = buffer;
	}
	return a;
}

void writeWavHeader(int bitdepth)
{
	wavStruct wavHeader;

	wavHeader.chunkSize = (MY_BUFFER_SIZE * NUM_OF_RUNS * bitdepth / 8 + 36);

	wavHeader.subChunk2Size = (MY_BUFFER_SIZE * NUM_OF_RUNS * bitdepth / 8);

	wavHeader.byteRate = SAMPLE_RATE * NUM_OF_CHANNELS * (bitdepth / 8); //== SampleRate * NumChannels * BitsPerSample/8
	wavHeader.blockAlign = NUM_OF_CHANNELS * bitdepth / 8;                                           //== NumChannels * BitsPerSample/8
	wavHeader.bitsPerSample = bitdepth;

	uFile.seek(0);
	if (!uFile.write((byte*)&wavHeader, 44) > 0)
	{
		// log msg
	}
}

void finalizeWav()
{
	unsigned long fSize = 0;
	fSize = uFile.size() - 8;
	uFile.seek(4);
	byte data[4] = { lowByte(fSize), highByte(fSize), fSize >> 16, fSize >> 24 };
	uFile.write(data, 4);
	byte tmp;
	uFile.seek(40);
	fSize = fSize - 36;
	data[0] = lowByte(fSize);
	data[1] = highByte(fSize);
	data[2] = fSize >> 16;
	data[3] = fSize >> 24;
	uFile.write((byte*)data, 4);
}