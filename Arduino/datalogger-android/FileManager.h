#pragma once

#include "config.h"
#include <SPI.h>
#include "SdFat.h"

/**
 * @file FileManager.h
 *
 * - connection to the sd-library using sdfat lib
 * - saves data given by the controller to the sd-card
 * - saves audio-data provided by the recorder
 *
 * @author Johannes Brunner
 *
 */

class FileManager {
private:

	const uint16_t MAX_NUM_OF_FOLDERS = 1000;
	uint16_t folderNumber_ = 0;
	char folderChar_[16];
	uint8_t fileOpened_ = false;
	uint8_t headerLineWritten = false;

	char* filename = "sensors.csv";

	SdFat sd; // File system object.

	SdFile file; // Log or audio file.

	const int SDchipSelect = 10;    // NANO_D10

	uint32_t cardSize = 0;

	uint32_t writtenBytes = 0;

	uint32_t const ERASE_SIZE = 262144L;
	uint32_t cardSectorCount = 0;
	uint8_t  sectorBuffer[512];

public:

	// initialize SD-card, better called only once
	void init();

	// file-functions:
	void openFile(char* filename = "sensors.csv");

	void closeFile(char* filename = "sensors.csv");

	void seekFile(uint32_t pos);

	uint32_t getFileSize();

	// datalog-functions:

	// save measurings in one line 
	void writeSensorsLine(measuring* data, uint8_t dataLength, char timeStamp[14] = "", int mode = -1);


	// audio-functions:

	void createAudioFile(char* filename);

	void writeAudioData(int16_t* audioData);

	void writeWavHeader(double recordingTime);

	void finalizeWav();


	// folder-funtions:

	uint16_t findFolderNumber();

	void createFolder();

	void enterFolder();

	void leaveFolder();

	// activte check

	uint8_t getOpenState();

	// power saving function

	void clearSPI();

	// error function
	void showError();

	void eraseCard();

	void formatCard();

	uint32_t getCardSize();

	uint32_t getWrittenBytes();

	FileManager() {};
	~FileManager() {};
};

