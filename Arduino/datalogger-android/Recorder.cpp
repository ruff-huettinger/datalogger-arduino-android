#include "Recorder.h"

int16_t Recorder::sampleBuffer_[BUFFER_SIZE];
volatile int16_t Recorder::bufferFilled_ = 0;

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
	PDM.read(sampleBuffer_, bytesAvailable);

	// 16-bit, 2 bytes per sample
	bufferFilled_ = bytesAvailable / 2;
}

int16_t* Recorder::getRecordedSamples()
{
	return sampleBuffer_;
}


void Recorder::stop()
{
	PDM.end();
	isRecording_ = false;
}

uint8_t Recorder::isBufferFull()
{
	return bufferFilled_;
}

void Recorder::setBufferEmpty()
{
	bufferFilled_ = 0;
}

uint8_t Recorder::isRecording() {
	return isRecording_;
}





