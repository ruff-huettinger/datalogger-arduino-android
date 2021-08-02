#include "Recorder.h"

int16_t Recorder::sampleBuffer[BUFFER_SIZE];
volatile int16_t Recorder::bufferFilled = 0;

void Recorder::init()
{
	PDM.onReceive(onPDMdata);

	PDM.setBufferSize(BUFFER_SIZE * 2);
}

void Recorder::begin()
{
	isRecording_ = true;
	if (!PDM.begin(NUM_OF_CHANNELS, SAMPLE_RATE))
	{
			DEBUG_PRINTLN("Failed to start PDM!");
			return;
	}
	// optionally set the gain, defaults to 20
	PDM.setGain(GAIN);
}

void Recorder::onPDMdata() {
	// query the number of bytes available
	int bytesAvailable = PDM.available();

	// read into the sample buffer
	PDM.read(sampleBuffer, bytesAvailable);

	// 16-bit, 2 bytes per sample
	bufferFilled = bytesAvailable / 2;
}

int16_t* Recorder::getRecordedSamples()
{
	return sampleBuffer;
}


void Recorder::stop()
{
	PDM.end();
	isRecording_ = false;
}

bool Recorder::isBufferFull()
{
	return bufferFilled;
}

void Recorder::setBufferEmpty()
{
	bufferFilled = 0;
}


/*
uint8_t Recorder::checkAmbienceVolume() {
	if (bufferFilled) {
		uint32_t bufferRMS = computeRMSValue();
		bufferFilled = 0;

	#ifdef DEBUG_AUDIO
		DEBUG_PRINT("RMS: ");
		DEBUG_PRINTLN(bufferRMS);
	#endif // DEBUG_AUDIO

		if (bufferRMS > MIC_ACTIVE_THRESHOLD && bufferCount > NUM_OF_IGNORED_BUFFERS) {
			loudEnough = true;
			return 1;
		}
		bufferCount++;
		if (bufferCount < NUM_OF_TEST_BUFFERS) {
			// continue sampling
			return 0;
		}
		else {
			// last test buffer no success -> stop sampling
			loudEnough = false;
			return 2;
		}
	}
	else {
		return 0;
	}
}

uint32_t Recorder::computeRMSValue()
{
	int32_t bufferRMS = 0;

	for (int i = 0; i < bufferFilled; i++) {
		bufferRMS += sq(sampleBuffer[i]);
	}
	bufferRMS = bufferRMS / bufferFilled;
	bufferRMS = sqrt(bufferRMS);

	return bufferRMS;
}
*/

bool Recorder::isRecording() {
	return isRecording_;
}





