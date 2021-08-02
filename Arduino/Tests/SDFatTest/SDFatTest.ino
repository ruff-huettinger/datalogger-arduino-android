/*
  SDFatTest

  - A test to check connection to sd-card (module)
  - Adaption of example from bill greimans sdfat-lib


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Sd card on cs pin 2
  * 
  Created 03 11 2020
  By Johannes Brunner  
*/

#include <SPI.h>
#include "SdFat.h"

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 2;

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 3000;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "sdfat"
//------------------------------------------------------------------------------
// File system object.
SdFat sd;

// Log file.
SdFile file;

// Time in micros for next data record.
uint32_t logTime;

//==============================================================================
// User functions.  Edit writeHeader() and logData() for your requirements.


//------------------------------------------------------------------------------
// Write data header.
void writeHeader() {
  file.println("head");
}
//------------------------------------------------------------------------------
// Log a data record.
void logData() {
  // Write data to file.  Start with log time in micros.
  file.print(logTime);

  file.write(',');
  file.print("test");

  file.println();
}
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
//------------------------------------------------------------------------------
void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial && millis() < 3000);
  delay(1000);

  Serial.println(F("Type any character to start"));
  /*
  while (!Serial.available()) {
    SysCall::yield();
  }*/

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error("file.open");
  }
  // Read any Serial data.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  Serial.println(F("Type any character to stop"));

  // Write data header.
  writeHeader();

  // Start on a multiple of the sample interval.
  logTime = micros() / (1000UL * SAMPLE_INTERVAL_MS) + 1;
  logTime *= 1000UL * SAMPLE_INTERVAL_MS;
}
//------------------------------------------------------------------------------
void loop() {
  // Time for next record.
  logTime += 1000UL * SAMPLE_INTERVAL_MS;

  // Wait for log time.
  int32_t diff;
  do {
    diff = micros() - logTime;
  } while (diff < 0);

  // Check for data rate too high.
  if (diff > 10) {
    error("Missed data record");
  }

  logData();

  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    error("write error");
  }

  
    file.close();
    Serial.println(F("Done"));
    digitalWrite(LED_PWR, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    SysCall::halt();
}
