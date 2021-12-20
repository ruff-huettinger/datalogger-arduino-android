#pragma once

#include "config.h"
#include <SPI.h>
#include "SdFat_2_1.h"

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
	uint32_t const ERASE_SIZE = 262144L;
	const char* FILE_NAME = "sensors.csv";
	const uint8_t SD_CHIP_SELECT = 10;    // NANO_D10

	uint16_t folderNumber_ = 0;
	char folderChar_[16];
	uint8_t fileOpened_ = false;
	uint8_t headerLineWritten_ = false;
	uint32_t cardSize_ = 0;
	uint32_t writtenBytes_ = 0;
	uint32_t cardSectorCount_ = 0;
	uint8_t  sectorBuffer_[512];
	uint8_t sdInitialized_ = false;
	uint16_t manufacturerID_ = 0;

	SdFat sd_;
	SdFile file_;

public:
	// initialize SD-card, better called only once
	void init();

	// file-functions:
	void openFile(char* filename = "sensors.csv");
	void closeFile(char* filename = "sensors.csv");
	void seekFile(uint32_t pos);
	uint32_t getFileSize();

	// datalog-function:
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

	// format functions:
	void getCardSectorCount();
	void eraseCard();
	void formatCard();

	uint8_t checkSDIntegrity();
	uint8_t getOpenState();
	void clearSPI();
	void showError();
	uint32_t getCardSize();
	uint32_t getWrittenBytes();

	FileManager() {};
	~FileManager() {};
};

