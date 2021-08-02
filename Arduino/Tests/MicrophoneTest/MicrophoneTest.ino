/*
  MicrophoneTest

  - older Version of MicrophoneTestRecording.ino
  - only works for 8000hz or less
  - this implementation was used by the egg-prototyp

  Created 06 11 2020
  By Johannes Brunner
  
*/


#include <PDM.h>
#include <SPI.h>
#include <SD.h>


#define RECORD_DEBUG

const uint16_t BUFFER_SIZE = 16384;

const uint16_t NUM_OF_BUFFERS = 40;

const uint16_t SAMPLE_RATE = 16000;

const uint8_t BIT_DEPTH = 16;

const uint8_t NUM_OF_CHANNELS = 1;

const uint8_t GAIN = 10;

/* WAV HEADER STRUCTURE */
struct wavStruct {
	const char chunkID[4] = { 'R','I','F','F' };
	const uint32_t chunkSize = (BUFFER_SIZE / 2 * NUM_OF_BUFFERS + 36); //Size of (entire file in bytes - 8 bytes) or (data size + 36)
	const char format[4] = { 'W','A','V','E' };
	const char subchunkID[4] = { 'f','m','t',' ' };
	const uint32_t subchunkSize = 16;
	const uint16_t audioFormat = 1;              //PCM == 1
	const uint16_t numChannels = NUM_OF_CHANNELS;                    //1=Mono, 2=Stereo
	const uint32_t sampleRate = SAMPLE_RATE / 2;
	const uint32_t byteRate = SAMPLE_RATE * NUM_OF_CHANNELS * BIT_DEPTH / 8; //== SampleRate * NumChannels * BitsPerSample/8
	const uint16_t blockAlign = 2;                     //== NumChannels * BitsPerSample/8
	const uint16_t bitsPerSample = BIT_DEPTH;                  //8,16,32...
	const char subChunk2ID[4] = { 'd','a','t','a' };
	const uint32_t subChunk2Size = BUFFER_SIZE / 2 * NUM_OF_BUFFERS;                  //== NumSamples * NumChannels * BitsPerSample/8
	//Data                                       //The audio data
};

/*********************************************************/


// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

// number of samples read
volatile int samplesRead;

const int chipSelect = 10;

bool finalized = false;

char* filename = "myWav.wav"; //The name of the file that will be generated

int cnt = 0;


void setup() {
	Serial.begin(9600);
	while (!Serial);

	// configure the data receive callback
	PDM.onReceive(onPDMdata);

	// optionally set the gain, defaults to 20
	PDM.setGain(GAIN);

	PDM.setBufferSize(BUFFER_SIZE);

	// initialize PDM with:
	// - one channel (mono mode)
	// - a 16 kHz sample rate
	if (!PDM.begin(NUM_OF_CHANNELS, SAMPLE_RATE)) {
		Serial.println("Failed to start PDM!");
		while (1);
	}

	Serial.print("Initializing SD card...");

	if (!SD.begin(chipSelect)) {
		Serial.println("initialization failed. Things to check:");
		Serial.println("1. is a card inserted?");
		Serial.println("2. is your wiring correct?");
		Serial.println("3. did you change the chipSelect pin to match your shield or module?");
		Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
		while (true);
	}

	Serial.println("initialization done.");

	createWavHeader(filename, SAMPLE_RATE / 2);
}

void loop() {
	// wait for samples to be read
	if (samplesRead && !finalized) {

		Serial.println(samplesRead);

		/*
		// print samples to the serial monitor or plotter
		for (int i = 0; i < samplesRead; i++) {
			Serial.println(sampleBuffer[i]);
		}*/

		//writeSimpleDigitalTones();

		writeTones();

		// clear the read count
		samplesRead = 0;

		if (cnt >= NUM_OF_BUFFERS) {
			finalizeWavTemplate(filename);
			Serial.println("finished");
			finalized = true;
		}
		cnt++;
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

void createWavHeader(const char* fileName, uint32_t sampleRate) {

	if (!SD.exists(fileName)) {
	#if defined (RECORD_DEBUG)
		Serial.println("File does not exist, please write WAV/PCM data starting at byte 44");
	#endif
		return;
	}
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

#if defined (RECORD_DEBUG)
	Serial.print("WAV Header Write ");
#endif

	recFile.seek(0);
	if (recFile.write((byte*)&wavHeader, 44) > 0) {
	#if defined (RECORD_DEBUG)    
		Serial.println("OK");
	}
	else {
		Serial.println("Failed");
	#endif
	}
	recFile.close();

}

void writeTones() {
	/*Standard SD Lib */
	File uFile = SD.open(filename, FILE_WRITE);
	if (!uFile) { Serial.print("Failed to open"); return; }

	// write samples to the sd
	for (int i = 0; i < samplesRead; i++) {
		uFile.write(sampleBuffer[i]);
	}

	uFile.close();
	Serial.println("buffer written");
}

void writeSimpleDigitalTones() {
	/*Standard SD Lib */
	File uFile = SD.open(filename, FILE_WRITE);
	if (!uFile) { Serial.print("Failed to open"); return; }

	/*SDFAT Lib*/
  //  uFile.open(filename, O_WRITE);
  //  uFile.seekSet(44);
  // if(!uFile.isOpen()){ Serial.print("Failed to open"); return; }  

  //  Generate some waves, ramping up then down 6 times and write the data to the wav file
	int rampVal = 25;

	for (int g = 0; g < 6; g++) {                 //Do this 3 times
		for (int h = 0; h < 1000; h++) {             //Do this 100 Times
			for (int i = 1; i < 254; i += rampVal) {  //Count from 1 to 254, stepping up by rampVal
				uFile.write(i);               //Write the ramping values to the wav file
			}
			for (int i = 254; i > 10; i -= rampVal) { //Ramp the values down again
				uFile.write(i);               //Write the ramping values to the wav file
			}
		}
		Serial.print("Wrote "); Serial.print(g); Serial.println(" of 5");
		rampVal = rampVal + 20;    //Adjust the ramping value to create different tones
	}

	uFile.close();
	Serial.print("WAV file written, ready to finalize.");
}

void finalizeWavTemplate(char* filename) {

	unsigned long fSize = 0;

	File sFile = SD.open(filename, FILE_WRITE);

	if (!sFile) {
	#if defined (debug)
		Serial.println("fl");
	#endif
		return;
	}
	fSize = sFile.size() - 8;



	sFile.seek(4); byte data[4] = { lowByte(fSize),highByte(fSize), fSize >> 16,fSize >> 24 };
	sFile.write(data, 4);
	byte tmp;
	sFile.seek(40);
	fSize = fSize - 36;
	data[0] = lowByte(fSize); data[1] = highByte(fSize); data[2] = fSize >> 16; data[3] = fSize >> 24;
	sFile.write((byte*)data, 4);
	sFile.close();

	Serial.println("finalized");


}
