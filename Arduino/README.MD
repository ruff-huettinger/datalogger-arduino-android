# Electronic Egg Arduino Software



## Description

This directory contains the microcontroller software for the Electronic Egg developed by Kurt Hüttinger GmbH.

## Folder structure

    .
    ├── datalogger-android    	# Source code running on electronic egg
    ├── libraries               # (modified) libraries
    ├── Tests             		# Some tests for single components/functionalities
    ├── LICENSE
    └── README.md

### Hardware
* Arduino Nano 33 BLE Sense
* Electronic Egg (manufactured by Hüttinger)

### Dependencies

* Arduino Board Version: "[DEPRECATED] Arduino Mbed OS Boards 1.3.2"  
* Libraries provided in "libraries" subfolder

### Installing

* Install Arduino IDE Version 1.8.13
* Set top-level folder as sketchbook-location in IDE
* Compile "datalogger-android.ino" and upload 

* Alternative: Visual Micro or Platform.io

### Tasks to improve
* Audio quality
* BLE range
* Humidity measurements

### Ideas
* Use mbedOs Framework instead of Arduino
* Build a BLE Repeater for higher distances
* Change BLE to Wifi Connectivity

