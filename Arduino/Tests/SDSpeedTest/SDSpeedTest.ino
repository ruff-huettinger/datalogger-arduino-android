
#define SD_CS_PIN 10
#define TEST_FILE_NAME "testfile.txt"
// 1024 block file
#define FILE_SIZE 1024UL*512UL
#include <SPI.h>


// use next line to test SD.h
//#include <SD.h>

// use next two lines to test SdFat
#include "SdFat_2_1.h"
SdFat SD;

uint8_t buf[512];
File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  pinMode(3, OUTPUT);
  delay(5);
  digitalWrite(3, LOW);
  delay(5);


  // Wait for USB Serial
  while (!Serial) {

  }
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD.begin failed!");
    return;
  }


  Serial.println("begin free space test");
  //ShowFreeSpace();


  for (size_t n = 1; n <= 512; n *= 2) {
    SD.remove(TEST_FILE_NAME);
    myFile = SD.open(TEST_FILE_NAME, FILE_WRITE);
    if (!myFile) {
      Serial.println("open failed");
      return;
    }

    // Write file data
    uint32_t us = micros();
    for (uint32_t i = 0; i < FILE_SIZE; i += n) {
      if (n != myFile.write(buf, n)) {
        Serial.println("Write failed");
        return;
      }
    }
    us = micros() - us;
    myFile.close();

    Serial.print("buffer size (bytes): ");
    Serial.print(n);
    Serial.print(", time (sec): ");
    Serial.print(0.000001*us);
    Serial.print(", rate (KB/sec): ");
    Serial.println(FILE_SIZE / (0.001 * us));
  }
}

void loop() {
  // nothing happens after setup
}

/*
void ShowFreeSpace() {
    // Calculate free space (volume free clusters * blocks per clusters / 2)
    long lFreeKB = SD.vol()->freeClusterCount();
    lFreeKB *= SD.vol()->blocksPerCluster() / 2;

    // Display free space
    Serial.print("Free space: ");
    Serial.print(lFreeKB);
    Serial.println(" KB");
}
*/
