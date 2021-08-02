/*
  ShieldTest

  - A test to check connection to Arduinos sd-card module
  - Could also be used for flash tests
  - Adaption of an example from Arduino's SD-lib (which is based on greiman's sdfat-lib)

  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Sd card on cs pin 2
  
  Created 10 08 2020
  By Johannes Brunner  

*/

#include "SerialFlash.h"
#include "SD.h"
#include <SPI.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// file name to use for writing
const char filename[] = "datalog.txt";


// File object to represent file
File myFile;


const int SDchipSelect = 2;    // NANO_D2
const int FlashChipSelect = 3; // NANO_D3
//const int FlashChipSelect = 21; // Arduino 101 built-in SPI Flash

void setup() {

	Serial.begin(9600);

	// wait up to 10 seconds for Arduino Serial Monitor
	unsigned long startMillis = millis();
	while (!Serial && (millis() - startMillis < 10000));
	delay(100);

 /*

	if (!SerialFlash.begin(FlashChipSelect)) {
		printError("Unable to access SPI Flash chip");
	}*/
	if (!SD.begin(SDchipSelect)) {
		printError("Unable to access SD card");
	}
	printCardInfo();
	//toDo: fix copy to flash
	//copyToFlash();
	writeToCard();
	readFromCard();
	delay(100);



}


bool compareFiles(File& file, SerialFlashFile& ffile) {
	file.seek(0);
	ffile.seek(0);
	unsigned long count = file.size();
	while (count > 0) {
		char buf1[128], buf2[128];
		unsigned long n = count;
		if (n > 128) n = 128;
		file.read(buf1, n);
		ffile.read(buf2, n);
		if (memcmp(buf1, buf2, n) != 0) return false; // differ
		count = count - n;
	}
	return true;  // all data identical
}


void loop() {
}

void printError(const char* message) {
	while (1) {
		Serial.println(message);
		delay(2500);
	}
}

void printCardInfo() {
	Serial.print("\nInitializing SD card...");

	// we'll use the initialization code from the utility libraries
	// since we're just testing if the card is working!
	if (!card.init(SPI_HALF_SPEED, SDchipSelect)) {
		Serial.println("initialization failed. Things to check:");
		Serial.println("* is a card inserted?");
		Serial.println("* is your wiring correct?");
		Serial.println("* did you change the chipSelect pin to match your shield or module?");
		while (1);
	}
	else {
		Serial.println("Wiring is correct and a card is present.");
	}
	// print the type of card
	Serial.println();
	Serial.print("Card type:         ");
	switch (card.type()) {
	case SD_CARD_TYPE_SD1:
		Serial.println("SD1");
		break;
	case SD_CARD_TYPE_SD2:
		Serial.println("SD2");
		break;
	case SD_CARD_TYPE_SDHC:
		Serial.println("SDHC");
		break;
	default:
		Serial.println("Unknown");
	}

	// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
	if (!volume.init(card)) {
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
		while (1);
	}

	Serial.print("Clusters:          ");
	Serial.println(volume.clusterCount());
	Serial.print("Blocks x Cluster:  ");
	Serial.println(volume.blocksPerCluster());

	Serial.print("Total Blocks:      ");
	Serial.println(volume.blocksPerCluster() * volume.clusterCount());
	Serial.println();

	// print the type and size of the first FAT-type volume
	uint32_t volumesize;
	Serial.print("Volume type is:    FAT");
	Serial.println(volume.fatType(), DEC);

	volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
	volumesize *= volume.clusterCount();       // we'll have a lot of clusters
	volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
	Serial.print("Volume size (Kb):  ");
	Serial.println(volumesize);
	Serial.print("Volume size (Mb):  ");
	volumesize /= 1024;
	Serial.println(volumesize);
	Serial.print("Volume size (Gb):  ");
	Serial.println((float)volumesize / 1024.0);

	Serial.println("\nFiles found on the card (name, date and size in bytes): ");
	root.openRoot(volume);

	// list all files in the card with date and size
	root.ls(LS_R | LS_DATE | LS_SIZE);
	root.close();
}

void copyToFlash() {
	File rootdir = SD.open("/");
	while (1) {
		// open a file from the SD card
		Serial.println();
		File f = rootdir.openNextFile();
		if (!f) break;
		const char* filename = f.name();
		Serial.print(filename);
		Serial.print("    ");
		unsigned long length = f.size();
		Serial.println(length);

		// check if this file is already on the Flash chip
		if (SerialFlash.exists(filename)) {
			Serial.println("  already exists on the Flash chip");
			SerialFlashFile ff = SerialFlash.open(filename);
			if (ff && ff.size() == f.size()) {
				Serial.println("  size is the same, comparing data...");
				if (compareFiles(f, ff) == true) {
					Serial.println("  files are identical :)");
					f.close();
					ff.close();
					continue;  // advance to next file
				}
				else {
					Serial.println("  files are different");
				}
			}
			else {
				Serial.print("  size is different, ");
				Serial.print(ff.size());
				Serial.println(" bytes");
			}
			// delete the copy on the Flash chip, if different
			Serial.println("  delete file from Flash chip");
			SerialFlash.remove(filename);
		}

		// create the file on the Flash chip and copy data
		if (SerialFlash.create(filename, length)) {
			SerialFlashFile ff = SerialFlash.open(filename);
			if (ff) {
				Serial.print("  copying");
				// copy data loop
				unsigned long count = 0;
				unsigned char dotcount = 9;
				while (count < length) {
					char buf[256];
					unsigned int n;
					n = f.read(buf, 256);
					ff.write(buf, n);
					count = count + n;
					Serial.print(".");
					if (++dotcount > 100) {
						Serial.println();
						dotcount = 0;
					}
				}
				ff.close();
				if (dotcount > 0) Serial.println();
			}
			else {
				Serial.println("  error opening freshly created file!");
			}
		}
		else {
			Serial.println("  unable to create file");
		}
		f.close();
	}
	rootdir.close();
	delay(10);
	Serial.println("Finished All Files");
}

void writeToCard() {
	// If you want to start from an empty file,
	// uncomment the next line:
	//SD.remove(filename);
	// try to open the file for writing

	myFile = SD.open(filename, FILE_WRITE);
	if (!myFile) {
		Serial.print("error opening ");
		Serial.println(filename);
		while (true);
	}

	// add some new lines to start
	myFile.println();
	myFile.println("Hello World!");
	myFile.close();

	Serial.println("Finished writing to file...");
}

void readFromCard() {
	// re-open the file for reading:
	myFile = SD.open(filename, FILE_READ);
	if (myFile) {
		Serial.println("filename");

		// read from the file until there's nothing else in it:
		while (myFile.available()) {
			Serial.write(myFile.read());
		}
		// close the file:
		myFile.close();
	}
	else {
		// if the file didn't open, print an error:
		Serial.println("error opening test.txt");
	}
}
