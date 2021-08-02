/*
  MicrophoneRecordingTest

  - A test to record Audio directly on a sd-card
  - Works well with Arduino Nano's built-in microphone and the right sampling frequency


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Micro-SD card connected via SPI and CS on pin 10

  Results for 15625 samples:
  * Time ratio Sampling vs Writing (sdfat!): ~4/3 -> writing is fast enough
  * Buffer size must be at least 1024 for less noise

  Created 09 12 2020
  By Johannes Brunner

*/

#include <PDM.h>
#include <SPI.h>
#include "SdFat.h"

//#define LOG_TIMES
#define RECORD_DEBUG

const uint16_t BUFFER_SIZE = 1024;

const uint16_t NUM_OF_BUFFERS = 0;

const uint16_t NUM_OF_RUNS = 1000;

// use 16000 31250 41667
const uint16_t SAMPLE_RATE = 41667;


const uint8_t BIT_DEPTH = 8;

const uint8_t NUM_OF_CHANNELS = 1;

const uint8_t GAIN = 10;

char* filename = "myWavMusic41k8bit.wav"; //The name of the file that will be generated

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

short holdingBuffer[4 * BUFFER_SIZE];

// number of samples read
volatile int samplesRead;

const int chipSelect = 10;

bool finalized = false;


int cnt = 0;

int maxCnt = 0;

uint32_t time1 = 0;

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

	createWavHeader(filename, SAMPLE_RATE);

	uFile = SD.open(filename, FILE_WRITE);

	if (!uFile)
	{
		Serial.print("Failed to open");
		return;
	}
}

void loop()
{
	// wait for samples to be read

	if (samplesRead && !finalized)
	{

		uint32_t curTime = millis();

		//Serial.println(samplesRead);

	#ifdef LOG_TIMES
		Serial.print("Sampling time 12000: ");
		Serial.println(curTime - time1);
		time1 = millis();
	#endif // LOG_TIMES

		//writeSimpleDigitalTones();

		//writeTones();

		//holdInBuffer();

		// clear the read count
		samplesRead = 0;

		cnt++;
		if (cnt > NUM_OF_BUFFERS)
		{
		#ifdef LOG_TIMES
			uint32_t timeBefSD = millis();
		#endif // LOG_TIMES

			saveHoldBufferToSD();

		#ifdef LOG_TIMES
			Serial.print("SD time: ");
			Serial.println(millis() - timeBefSD);
		#endif // LOG_TIMES

			/*
			for (int i = 0; i < 4 * BUFFER_SIZE; i++)
			{
				Serial.print(i);
				Serial.print(" : ");
				Serial.println(holdingBuffer[i]);
			}
			*/

			//Serial.println("finished");
			maxCnt++;
			if (maxCnt >= NUM_OF_RUNS)
			{
				Serial.println("done");
				finalized = true;
				uFile.close();
				finalizeWavTemplate(filename);
			}
			cnt = 0;
		}

		//time1 = millis();
	}
}

void holdInBuffer()
{
	/*
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		holdingBuffer[cnt * BUFFER_SIZE + i] = sampleBuffer[i];
	}*/
	memcpy(&holdingBuffer[cnt * BUFFER_SIZE], &sampleBuffer[0], BUFFER_SIZE * sizeof(sampleBuffer[0]));
}

void saveHoldBufferToSD()
{
	uint32_t timeBefWrite = millis();
	int8_t* x[BUFFER_SIZE];

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		// clamp 16bit to -127/+127 with midpoint 0 for 8 bit .wav
		constrain(sampleBuffer[i], -127, 127);
		sampleBuffer[i] += 128;
		//x[i] = (int8_t*)(lowByte(sampleBuffer[i]));
		//x[i] = &((int8_t)((sampleBuffer[i]) & 0xff));
		uFile.write(((sampleBuffer[i]) & 0xff));



		/*
		if (sampleBuffer[i] < -128) {
			x[i] = -128;
		}

		else if (sampleBuffer[i] > 127) {
			x[i] = 127;
		}

		else {
			x[i] = (sampleBuffer[i]);
		}*/

		//uFile.write(sampleBuffer[i]);
		//x[i * 2 + 1] = lowByte(sampleBuffer[i]);
		//x[i * 2] = highByte(sampleBuffer[i]);

		//x[i] = highByte(sampleBuffer[i]);

		//x[i] = (sampleBuffer[i] % 128);

		//uFile.write(sampleBuffer[i]);
		/*
		Serial.print(sampleBuffer[i]);
		Serial.print(" | ");
		Serial.print(x[i]);
		Serial.print(" | ");
		Serial.println(x[i + 1]);
		*/
	}

	/*
	if (!(uFile.write(*x, BUFFER_SIZE))) {
		Serial.println("write failed");
	};
	*/
	//uFile.write(sampleBuffer);

	// Write file data
	/*
	uint32_t us = micros();
	for (uint32_t i = 0; i < 1; i += n) {
		if (n != uFile.write(sampleBuffer, n)) {
			Serial.println("Write failed");
			return;
		}
	}*/
#ifdef LOG_TIMES
	Serial.print("Write time: ");
	Serial.println(millis() - timeBefWrite);
#endif // LOG_TIMES

	//uFile.close();
#ifdef LOG_TIMES

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

void createWavHeader(const char* fileName, uint32_t sampleRate)
{
	/*
	if (!SD.exists(fileName))
	{
	#if defined(RECORD_DEBUG)
		Serial.println("File does not exist, please write WAV/PCM data starting at byte 44");
	#endif
		return;
	}*/
	File recFile = SD.open(fileName, FILE_WRITE);

	/*
	if (recFile.size() <= 44) {
	#if defined (RECORD_DEBUG)
		Serial.println("File contains no data, exiting");
	#endif
		recFile.close();
		return;
	}*/

	wavStruct wavHeader;
	//wavHeader.chunkSize = recFile.size() - 8;
	//wavHeader.numChannels = numChannels;
	//wavHeader.sampleRate = sampleRate;
	//wavHeader.byteRate = sampleRate * wavHeader.numChannels * wavHeader.bitsPerSample / 8;
	//wavHeader.blockAlign = wavHeader.numChannels * wavHeader.bitsPerSample / 8;
	//wavHeader.bitsPerSample = bitsPerSample;
	//wavHeader.subChunk2Size = wavHeader.chunkSize + 8 - 44;

#if defined(RECORD_DEBUG)
	Serial.print("WAV Header Write ");
#endif

	recFile.seek(0);
	if (recFile.write((byte*)&wavHeader, 44) > 0)
	{
	#if defined(RECORD_DEBUG)
		Serial.println("OK");
	}
	else
	{
		Serial.println("Failed");
	#endif
	}
	recFile.close();
}

void writeTones()
{
	/*Standard SD Lib */
	File uFile = SD.open(filename, FILE_WRITE);
	if (!uFile)
	{
		Serial.print("Failed to open");
		return;
	}

	// write samples to the sd
	for (int i = 0; i < samplesRead; i++)
	{
		uFile.write(sampleBuffer[i]);
	}

	uFile.close();
	Serial.println("buffer written");
}

void writeSimpleDigitalTones()
{
	/*Standard SD Lib */
	File uFile = SD.open(filename, FILE_WRITE);
	if (!uFile)
	{
		Serial.print("Failed to open");
		return;
	}

	/*SDFAT Lib*/
	//  uFile.open(filename, O_WRITE);
	//  uFile.seekSet(44);
	// if(!uFile.isOpen()){ Serial.print("Failed to open"); return; }

	//  Generate some waves, ramping up then down 6 times and write the data to the wav file
	int rampVal = 25;

	for (int g = 0; g < 6; g++)
	{ //Do this 3 times
		for (int h = 0; h < 1000; h++)
		{ //Do this 100 Times
			for (int i = 1; i < 254; i += rampVal)
			{                   //Count from 1 to 254, stepping up by rampVal
				uFile.write(i); //Write the ramping values to the wav file
			}
			for (int i = 254; i > 10; i -= rampVal)
			{                   //Ramp the values down again
				uFile.write(i); //Write the ramping values to the wav file
			}
		}
		Serial.print("Wrote ");
		Serial.print(g);
		Serial.println(" of 5");
		rampVal = rampVal + 20; //Adjust the ramping value to create different tones
	}

	uFile.close();
	Serial.print("WAV file written, ready to finalize.");
}

void finalizeWavTemplate(char* filename)
{

	unsigned long fSize = 0;

	File sFile = SD.open(filename, FILE_WRITE);

	if (!sFile)
	{
	#if defined(debug)
		Serial.println("fl");
	#endif
		return;
}
	fSize = sFile.size() - 8;

	sFile.seek(4);
	byte data[4] = { lowByte(fSize), highByte(fSize), fSize >> 16, fSize >> 24 };
	sFile.write(data, 4);
	byte tmp;
	sFile.seek(40);
	fSize = fSize - 36;
	data[0] = lowByte(fSize);
	data[1] = highByte(fSize);
	data[2] = fSize >> 16;
	data[3] = fSize >> 24;
	sFile.write((byte*)data, 4);
	sFile.close();

	Serial.println("finalized");
}
