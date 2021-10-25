#include "FileManager.h"

void FileManager::init()
{
	// sd.begin should only be called once, because it causes a memory leak...
	if (!sd.begin(SDchipSelect, SD_SCK_MHZ(50))) {
		DEBUG_PRINTLN("!!!- Unable to access SD card -!!!");
	#ifndef DEBUG_BOARD
		//showError();
		sdInitialized = false;
	#endif // !DEBUG_BOARD
	}
	else {
		sdInitialized = true;
	}

	csd_t m_csd;
	!sd.card()->readCSD(&m_csd);
	cardSize = 0.000512 * sdCardCapacity(&m_csd);

	cid_t m_cid;
	sd.card()->readCID(&m_cid);
	manufacturerID = int(m_cid.mid);
}

uint16_t FileManager::findFolderNumber()
{
	char buffer[16];
	int a = 0;
	sprintf(buffer, "%d", a);
	const char* x = buffer;
	while (sd.exists(x) && a < MAX_NUM_OF_FOLDERS) {
		a++;
		sprintf(buffer, "%d", a);
		x = buffer;
	}
	return a;
}

void FileManager::openFile(char* filename)
{
	enterFolder();
	if (!file.open(filename, O_WRONLY | O_APPEND | O_CREAT)) {
		DEBUG_PRINT("error opening ");
		DEBUG_PRINTLN(filename);

		// close the connection the damaged file
		file.close();
		clearSPI();
		return;
	}
	fileOpened_ = true;
}

void FileManager::closeFile(char* filename)
{
	file.close();
	leaveFolder();
	fileOpened_ = false;

	clearSPI();
}


void FileManager::createAudioFile(char* filename)
{
	enterFolder();
	// creates new audio file
	if (!file.open(filename, O_WRONLY | O_CREAT)) {
		DEBUG_PRINT("error creating audio file:");
		DEBUG_PRINTLN(filename);
		file.close();
		return;
	}
	leaveFolder();
}

void FileManager::writeAudioData(int16_t* audioData)
{
	for (int i = 0; i < BUFFER_SIZE; i++) {
		if (BIT_DEPTH == 8) {

			// write 8-bit signed audio 

			uint16_t* unsignedData = (uint16_t*)(audioData[i] + 32767);
			file.write(*unsignedData >> 8);
			writtenBytes++;
		}
		else {
			//file.write(audioData[i]);
			file.write(lowByte(audioData[i]));
			file.write(highByte(audioData[i]));
		}
	}
}


void FileManager::seekFile(uint32_t pos)
{
	file.seekSet(pos);
}

uint32_t FileManager::getFileSize()
{
	return file.fileSize();
}

void FileManager::writeSensorsLine(measuring* data, uint8_t dataLength, char timeStamp[14], int mode)
{
	if (!headerLineWritten) {
		file.print("Time");
		file.print(",");
		for (int i = 0; i < dataLength; i++) {
			file.print(data[i].valueName);
			if (i != dataLength - 1) {
				file.print(",");
			}
			else {
			#ifdef DEBUG_MEMORY
				if (mode != -1) {
					file.print(",");
					file.print(",");
					file.print("Modus");
					file.print(",");
					file.print("Log");
				}
			#endif // DEBUG_MEMORY
				file.println();
			}
		}
		headerLineWritten = true;
		writtenBytes = writtenBytes + 7 + 4 * dataLength;
	}

	file.print(timeStamp);
	file.print(",");

	for (int i = 0; i < dataLength; i++) {
		//file.print(data[i].valueName);
		//file.print(":");
		file.print(data[i].value);
		if (i != dataLength - 1) {
			file.print(",");
		}
		else {
		#ifdef DEBUG_MEMORY
			if (mode != -1) {
				file.print(",");
				file.print(",");
				file.print(mode);
				file.print(",");
				file.print(freeMemory());
			}
		#endif // DEBUG_MEMORY
			file.println();
		}
	}

	writtenBytes = writtenBytes + 6 * dataLength;

	if (!file.sync() || file.getWriteError()) {
		DEBUG_PRINTLN("Writing sensors' line failed");
	}
}


void FileManager::writeWavHeader(double recordingTime)
{
	wavStruct wavHeader;
	wavHeader.chunkSize = wavHeader.sampleRate * (recordingTime / 1000) * (BIT_DEPTH / 8) + 36;
	wavHeader.subChunk2Size = wavHeader.sampleRate * (recordingTime / 1000) * (BIT_DEPTH / 8);

	file.seek(0);
	if (!file.write((byte*)&wavHeader, 44) > 0)
	{
		// log msg
	}
}

void FileManager::finalizeWav()
{
	unsigned long fSize = 0;
	fSize = file.size() - 8;
	file.seek(4);
	byte data[4] = { lowByte(fSize), highByte(fSize), fSize >> 16, fSize >> 24 };
	file.write(data, 4);
	byte tmp;
	file.seek(40);
	fSize = fSize - 36;
	data[0] = lowByte(fSize);
	data[1] = highByte(fSize);
	data[2] = fSize >> 16;
	data[3] = fSize >> 24;
	file.write((byte*)data, 4);
}

void FileManager::createFolder()
{
	folderNumber_ = findFolderNumber();
	sprintf(folderChar_, "%d", folderNumber_);

	const char* dir = folderChar_;
	// Create a new folder.
	if (!sd.exists(dir)) {
		if (!sd.mkdir(dir)) {
			DEBUG_PRINTLN("Creating Folder failed");
		}
	}
}

void FileManager::enterFolder()
{
	const char* dir = folderChar_;
	if (!sd.chdir(dir)) {
		DEBUG_PRINTLN("Entering folder failed");
	}
}

void FileManager::leaveFolder()
{
	const char* dir = folderChar_;
	if (!sd.chdir()) {
		DEBUG_PRINTLN("Leaving folder failed");
	}
}

byte FileManager::checkSDIntegrity()
{
	if (!sdInitialized) {
		return 2; // Error on SD Init
	}

	if (cardSize != 32010 || manufacturerID != 27) {
		return 3; // Not a original SD
	}

	return 0; // SD is genuine Samsung Pro
}

uint8_t FileManager::getOpenState()
{
	return fileOpened_;
}

uint32_t FileManager::getWrittenBytes()
{
	return writtenBytes;
}

void FileManager::clearSPI()
{
	sd.card()->forceNotBusy();
}

void FileManager::showError()
{
	pinMode(LED_RED, OUTPUT);
	while (true) {
		DEBUG_PRINTLN("!!!- No SD -!!!");
		digitalWrite(LED_RED, HIGH);   // turn the LED on (HIGH is the voltage level)
		delay(1000);                       // wait for a second
		digitalWrite(LED_RED, LOW);    // turn the LED off by making the voltage LOW
		delay(1000);                       // wait for a second
	}
}
void FileManager::getCardSectorCount()
{
	cardSectorCount = sd.card()->sectorCount();
}
void FileManager::eraseCard() {
	uint32_t firstBlock = 0;
	uint32_t lastBlock;
	uint16_t n = 0;

	do {
		lastBlock = firstBlock + ERASE_SIZE - 1;
		if (lastBlock >= cardSectorCount) {
			lastBlock = cardSectorCount - 1;
		}
		if (!(sd.card()->erase(firstBlock, lastBlock))) {
		}
		if ((n++) % 64 == 63) {
		}
		firstBlock += ERASE_SIZE;
	} while (firstBlock < cardSectorCount);

	if (!sd.card()->readSector(0, sectorBuffer)) {
	}
}

void FileManager::formatCard() {
	ExFatFormatter exFatFormatter;
	FatFormatter fatFormatter;

	// Format exFAT if larger than 32GB.
	bool rtn = cardSectorCount > 67108864 ?
		exFatFormatter.format(sd.card(), sectorBuffer, &Serial) :
		fatFormatter.format(sd.card(), sectorBuffer, &Serial);

	if (!rtn) {
	}
}

uint32_t FileManager::getCardSize()
{
	return cardSize;
}









