#pragma once

#include "PDMUpdate.h"
#include "config.h"

/**
 * @file Recorder.h
 *
 * - Records with microphone using arduino-pdm lib
 * - Gets audio settings from config file
 *
 * @author Johannes Brunner
 *
 */


class Recorder
{

private:
	static int16_t sampleBuffer_[BUFFER_SIZE];
	uint8_t isRecording_ = false;
	static volatile int16_t bufferFilled_;

	// callback-function for pdm-lib
	static void onPDMdata();

public:
	void init();
	void begin();
	void stop();

	uint8_t isBufferFull();
	void setBufferEmpty();
	uint8_t isRecording();

	// return pointer to recorded samples
	int16_t* getRecordedSamples();

	Recorder() {};
	~Recorder() {};
};
