/*
  Microphone8BitTest

  - A test to record 8bit-Audio with higher sampling frequency directly on a sd-card
  - Works well with Arduino Nano's built-in microphone, fast sd-cards and right settings
  - 16bit-audio-samples coming from Arduino's PDM library are clipped to 8bit-samples


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Micro-SD card connected via SPI and CS on pin 10

  Results for 31250 samples:
  * Sd-card speed: ~35kbps-45kbps -> works with 31250kbyte per second

  Created 05 01 2021
  By Johannes Brunner

*/

#include "PDMDup.h"
#include <SPI.h>
#include "SdFat.h"

//#define LOG_TIMES
#define RECORD_DEBUG

const uint16_t BUFFER_SIZE = 1024;

const uint16_t NUM_OF_BUFFERS = 0;

const uint16_t NUM_OF_RUNS = 1000;

// use 16000 31250 41667 -> best results with 31250
const uint16_t SAMPLE_RATE = 31250;

const uint8_t BIT_DEPTH = 8;

const uint8_t NUM_OF_CHANNELS = 1;

const uint8_t GAIN = 10;

char* filename = "myWav_31k_8bit.wav"; //The name of the file that will be generated

/* WAV HEADER STRUCTURE */
struct wavStruct
{
	const char chunkID[4] = { 'R', 'I', 'F', 'F' };
	const uint32_t chunkSize = (BUFFER_SIZE * NUM_OF_BUFFERS + 36); //Size of (entire file in bytes - 8 bytes) or (data size + 36)
	const char format[4] = { 'W', 'A', 'V', 'E' };
	const char subchunkID[4] = { 'f', 'm', 't', ' ' };
	const uint32_t subchunkSize = 16;
	const uint16_t audioFormat = 1;               //PCM == 1
	const uint16_t numChannels = NUM_OF_CHANNELS; //1=Mono, 2=Stereo
	const uint32_t sampleRate = SAMPLE_RATE;
	const uint32_t byteRate = SAMPLE_RATE * NUM_OF_CHANNELS * (BIT_DEPTH / 8); //== SampleRate * NumChannels * BitsPerSample/8
	const uint16_t blockAlign = NUM_OF_CHANNELS * BIT_DEPTH / 8;                                           //== NumChannels * BitsPerSample/8
	const uint16_t bitsPerSample = BIT_DEPTH;                                //8,16,32...
	const char subChunk2ID[4] = { 'd', 'a', 't', 'a' };
	const uint32_t subChunk2Size = BUFFER_SIZE * NUM_OF_BUFFERS; //== NumSamples * NumChannels * BitsPerSample/8
									 //Data                                       //The audio data
};

/*********************************************************/

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

// number of samples read
volatile int samplesRead;

const int chipSelect = 10;

bool finalized = false;

int cnt = 0;

int maxCnt = 0;

uint32_t timing = 0;

// File system object.
SdFat SD;
File uFile;

void setup()
{
	Serial.begin(9600);
	while (!Serial)
		;

	// configure the data receive callback
	PDM.onReceive(onPDMdata);

	// optionally set the gain, defaults to 20
	PDM.setGain(GAIN);

	// set Buffer to double size of samples buffer for lib's double buffering
	PDM.setBufferSize(BUFFER_SIZE * 2);

	// initialize PDM with:
	// - one channel (mono mode)
	// - sample rate
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

	createWavHeader();
}

void loop()
{
	// wait for a full audio buffer and callback 

	if (samplesRead && !finalized)
	{
	#ifdef LOG_TIMES
		uint32_t curTime = millis();
		Serial.print("Sampling time: ");
		Serial.println(curTime - timing);
		timing = millis();
	#endif // LOG_TIMES

		// clear the read count
		samplesRead = 0;

		cnt++;
		if (cnt > NUM_OF_BUFFERS)
		{
		#ifdef LOG_TIMES
			uint32_t timeBefSD = millis();
		#endif // LOG_TIMES

			saveSamplesOnSD();

		#ifdef LOG_TIMES
			Serial.print("SD time: ");
			Serial.println(millis() - timeBefSD);
		#endif // LOG_TIMES

			maxCnt++;
			if (maxCnt >= NUM_OF_RUNS)
			{
				Serial.println("done");
				finalized = true;
				finalizeWavTemplate();
				uFile.close();
			}
			cnt = 0;
		}
	}
}

void saveSamplesOnSD()
{
#ifdef LOG_TIMES
	uint32_t timeBefWrite = millis();
#endif // LOG_TIMES

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		// clamp 16bit to -127/+127 with midpoint 0 for 8 bit .wav
		constrain(sampleBuffer[i], -127, 127);

		// transform from int8 to uint8
		sampleBuffer[i] += 128;

		// write the lowbyte of the sample buffer to the sd card (0-255)
		uFile.write(((sampleBuffer[i]) & 0xff));
	}

#ifdef LOG_TIMES
	Serial.print("Write time: ");
	Serial.println(millis() - timeBefWrite);
	Serial.println("buffer written");
#endif // LOG_TIMES

}

void onPDMdata()
{
	// query the number of bytes available
	int bytesAvailable = PDM.available();

	// read into the sample buffer
	PDM.read(sampleBuffer, bytesAvailable);

	// 16-bit, 2 bytes per sample
	samplesRead = bytesAvailable / 2;
}

void createWavHeader()
{
	wavStruct wavHeader;

#if defined(RECORD_DEBUG)
	Serial.print("WAV Header Write ");
#endif

	uFile.seek(0);
	if (uFile.write((byte*)&wavHeader, 44) > 0)
	{
	#if defined(RECORD_DEBUG)
		Serial.println("OK");
	}
	else
	{
		Serial.println("Failed");
	#endif
	}
}

void finalizeWavTemplate()
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

	Serial.println("finalized");
}
