#pragma once

#include <ArduinoPDM.h>
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

	static int16_t sampleBuffer[BUFFER_SIZE];

	// number of already saved buffers
	uint16_t bufferCount = 0;

	bool isRecording_ = false;

	// flag for continueing sampling
	bool loudEnough = false;


	// states whether a new buffer was filled
	static volatile int16_t bufferFilled;

	// callback-function for pdm-lib
	static void onPDMdata();

	/* use volume to trigger audio recording
	*  currently not implemented
	uint8_t checkAmbienceVolume();
	uint32_t computeRMSValue();
	*/

public:

	// init one time
	void init();
	
	// start a recording
	void begin();
	
	// stop the pdm
	void stop();

	bool isBufferFull();

	void setBufferEmpty();

	bool isRecording();

	// return pointer to recorded samples
	int16_t* getRecordedSamples();

	Recorder() {};
	~Recorder() {};

};
