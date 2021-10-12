/*
  MicrophoneNoiseTest

  - A test to record 16bit-Audio with higher sampling frequency buffered on a sd-card
  - Record the same buffer in 8bit quality

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

const uint16_t NUM_OF_RUNS = 3;

// use 16000 31250 41667 -> best results with 31250
const uint16_t SAMPLE_RATE = 31250;

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


// buffer to read samples into, each sample is 16-bits
short myBuffer[MY_BUFFER_SIZE];
uint32_t bufferOffset = 0;
volatile bool bufferFullFlag = false;
int bytesAvailable = 0;
int numOfRunsCount = 0;

// File system object.
const int chipSelect = 10;
SdFat SD;
File uFile;

// toDo: - noise reduction -> does not work, too much noise on signal
//		 - dither test (using sox) -> try different dither algorithms
//       - increase spi speed

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
}

void loop()
{
	if (bufferFullFlag) {
		Serial.println("buffer is full");
		saveMyBufferOnSD(16, 0);
		saveMyBufferOnSD(8, 0);
		saveMyBufferOnSD(8, 1);
		saveMyBufferOnSD(8, 5);

		bufferOffset = 0;
		bufferFullFlag = false;
		numOfRunsCount++;
	}

	if (numOfRunsCount == NUM_OF_RUNS) {
		uFile.close();
		Serial.print("finished");
		while (true);
	}
}


void saveMyBufferOnSD(int bitdeph, int noiseGate) {
	char name[16];
	sprintf(name, "%d_%d.wav", bitdeph, noiseGate);
	Serial.println(name);

	uFile = SD.open(name, FILE_WRITE);

	if (numOfRunsCount == 0) {
		writeWavHeader(bitdeph);
	}

	if (!uFile)
	{
		Serial.print("Failed to open file");
		while (true)
			;
		return;
	}

	if (bitdeph == 16) {
		for (int i = 0; i < MY_BUFFER_SIZE; i++)
		{
			uFile.write((lowByte(myBuffer[i])));
			uFile.write((highByte(myBuffer[i])));
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

	if (numOfRunsCount == NUM_OF_RUNS - 1) {
		finalizeWav();
	}
	uFile.close();
	Serial.println("buffer was saved");
}


void onPDMdata()
{
	// query the number of bytes available
	bytesAvailable = PDM.available();

	if ((bufferOffset + bytesAvailable) < MY_BUFFER_SIZE) {
		// read into the my buffer
		PDM.read(&myBuffer[bufferOffset], bytesAvailable);
		bufferOffset += (bytesAvailable / 2);
	}
	else {
		bufferFullFlag = true;

		// read the data but dont use it
		bytesAvailable = PDM.available();
		int throwAwayBuffer[PDM_BUFFER_SIZE];
		PDM.read(&throwAwayBuffer[0], bytesAvailable);
	}
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


/*
void saveTheOldWay(const char* name) {
	uFile = SD.open(name, FILE_WRITE);

	if (!uFile)
	{
		Serial.print("Failed to open file");
		while (true)
			;
		return;
	}

	for (int i = 0; i < MY_BUFFER_SIZE; i++) {

		constrain(myBuffer[i], -127, 127);
		// transform from int8 to uint8
		myBuffer[i] += 128;
		uFile.write(((myBuffer[i]) & 0xff));
	}

	uFile.close();
	Serial.println("buffer was saved the old way");
}
*/
