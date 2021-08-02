/*****************************************************************
  LSM9DS1_Basic_I2C.ino
  SFE_LSM9DS1 Library Simple Example Code - I2C Interface
  Jim Lindblom @ SparkFun Electronics
  Original Creation Date: April 30, 2015
  https://github.com/sparkfun/LSM9DS1_Breakout

  The LSM9DS1 is a versatile 9DOF sensor. It has a built-in
  accelerometer, gyroscope, and magnetometer. Very cool! Plus it
  functions over either SPI or I2C.

  This Arduino sketch is a demo of the simple side of the
  SFE_LSM9DS1 library. It'll demo the following:
  How to create a LSM9DS1 object, using a constructor (global
  variables section).
  How to use the begin() function of the LSM9DS1 class.
  How to read the gyroscope, accelerometer, and magnetometer
  using the readGryo(), readAccel(), readMag() functions and
  the gx, gy, gz, ax, ay, az, mx, my, and mz variables.
  How to calculate actual acceleration, rotation speed,
  magnetic field strength using the calcAccel(), calcGyro()
  and calcMag() functions.
  How to use the data from the LSM9DS1 to calculate
  orientation and heading.

  Hardware setup: This library supports communicating with the
  LSM9DS1 over either I2C or SPI. This example demonstrates how
  to use I2C. The pin-out is as follows:
	LSM9DS1 --------- Arduino
	 SCL ---------- SCL (A5 on older 'Duinos')
	 SDA ---------- SDA (A4 on older 'Duinos')
	 VDD ------------- 3.3V
	 GND ------------- GND
  (CSG, CSXM, SDOG, and SDOXM should all be pulled high.
  Jumpers on the breakout board will do this for you.)

  The LSM9DS1 has a maximum voltage of 3.6V. Make sure you power it
  off the 3.3V rail! I2C pins are open-drain, so you'll be
  (mostly) safe connecting the LSM9DS1's SCL and SDA pins
  directly to the Arduino.

  Development environment specifics:
	IDE: Arduino 1.6.3
	Hardware Platform: SparkFun Redboard
	LSM9DS1 Breakout Version: 1.0

  This code is beerware. If you see me (or any other SparkFun
  employee) at the local, and you've found our code helpful,
  please buy us a round!

  Distributed as-is; no warranty is given.
*****************************************************************/
// The SFE_LSM9DS1 library requires both Wire1 and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h>
#include <SPI.h>
#include "SparkFunLSM9DS1.h"


/* biases
* 23:24:10.202 -> accel biases (mg)
23:24:10.202 -> -3.30
23:24:10.202 -> -25.51
23:24:10.202 -> -15.32
23:24:10.202 -> gyro biases (dps)
23:24:10.202 -> -0.97
23:24:10.202 -> -2.73
23:24:10.202 -> -0.01
23:24:10.202 -> Mag Calibration: Wave device in a figure eight until done!
23:24:27.785 -> Mag Calibration done!
23:24:27.785 -> mag biases (mG)
23:24:27.785 -> 501.46
23:24:27.785 -> 79.22
23:24:27.785 -> 15.75
*/

// Accelerometer and Gyroscope registers
#define LSM9DS1XG_ACT_THS      0x04
#define LSM9DS1XG_ACT_DUR     0x05
#define LSM9DS1XG_INT_GEN_CFG_XL    0x06
#define LSM9DS1XG_INT_GEN_THS_X_XL  0x07
#define LSM9DS1XG_INT_GEN_THS_Y_XL  0x08
#define LSM9DS1XG_INT_GEN_THS_Z_XL  0x09
#define LSM9DS1XG_INT_GEN_DUR_XL    0x0A
#define LSM9DS1XG_REFERENCE_G       0x0B
#define LSM9DS1XG_INT1_CTRL         0x0C
#define LSM9DS1XG_INT2_CTRL         0x0D
#define LSM9DS1XG_WHO_AM_I          0x0F  // should return 0x68
#define LSM9DS1XG_CTRL_REG1_G       0x10
#define LSM9DS1XG_CTRL_REG2_G       0x11
#define LSM9DS1XG_CTRL_REG3_G       0x12
#define LSM9DS1XG_ORIENT_CFG_G      0x13
#define LSM9DS1XG_INT_GEN_SRC_G     0x14
#define LSM9DS1XG_OUT_TEMP_L        0x15
#define LSM9DS1XG_OUT_TEMP_H        0x16
#define LSM9DS1XG_STATUS_REG        0x17
#define LSM9DS1XG_OUT_X_L_G         0x18
#define LSM9DS1XG_OUT_X_H_G         0x19
#define LSM9DS1XG_OUT_Y_L_G         0x1A
#define LSM9DS1XG_OUT_Y_H_G         0x1B
#define LSM9DS1XG_OUT_Z_L_G         0x1C
#define LSM9DS1XG_OUT_Z_H_G         0x1D
#define LSM9DS1XG_CTRL_REG4         0x1E
#define LSM9DS1XG_CTRL_REG5_XL      0x1F
#define LSM9DS1XG_CTRL_REG6_XL      0x20
#define LSM9DS1XG_CTRL_REG7_XL      0x21
#define LSM9DS1XG_CTRL_REG8         0x22
#define LSM9DS1XG_CTRL_REG9         0x23
#define LSM9DS1XG_CTRL_REG10        0x24
#define LSM9DS1XG_INT_GEN_SRC_XL    0x26
//#define LSM9DS1XG_STATUS_REG        0x27 // duplicate of 0x17!
#define LSM9DS1XG_OUT_X_L_XL        0x28
#define LSM9DS1XG_OUT_X_H_XL        0x29
#define LSM9DS1XG_OUT_Y_L_XL        0x2A
#define LSM9DS1XG_OUT_Y_H_XL        0x2B
#define LSM9DS1XG_OUT_Z_L_XL        0x2C
#define LSM9DS1XG_OUT_Z_H_XL        0x2D
#define LSM9DS1XG_FIFO_CTRL         0x2E
#define LSM9DS1XG_FIFO_SRC          0x2F
#define LSM9DS1XG_INT_GEN_CFG_G     0x30
#define LSM9DS1XG_INT_GEN_THS_XH_G  0x31
#define LSM9DS1XG_INT_GEN_THS_XL_G  0x32
#define LSM9DS1XG_INT_GEN_THS_YH_G  0x33
#define LSM9DS1XG_INT_GEN_THS_YL_G  0x34
#define LSM9DS1XG_INT_GEN_THS_ZH_G  0x35
#define LSM9DS1XG_INT_GEN_THS_ZL_G  0x36
#define LSM9DS1XG_INT_GEN_DUR_G     0x37
//
// Magnetometer registers
#define LSM9DS1M_OFFSET_X_REG_L_M   0x05
#define LSM9DS1M_OFFSET_X_REG_H_M   0x06
#define LSM9DS1M_OFFSET_Y_REG_L_M   0x07
#define LSM9DS1M_OFFSET_Y_REG_H_M   0x08
#define LSM9DS1M_OFFSET_Z_REG_L_M   0x09
#define LSM9DS1M_OFFSET_Z_REG_H_M   0x0A
#define LSM9DS1M_WHO_AM_I           0x0F  // should be 0x3D
#define LSM9DS1M_CTRL_REG1_M        0x20
#define LSM9DS1M_CTRL_REG2_M        0x21
#define LSM9DS1M_CTRL_REG3_M        0x22
#define LSM9DS1M_CTRL_REG4_M        0x23
#define LSM9DS1M_CTRL_REG5_M        0x24
#define LSM9DS1M_STATUS_REG_M       0x27
#define LSM9DS1M_OUT_X_L_M          0x28
#define LSM9DS1M_OUT_X_H_M          0x29
#define LSM9DS1M_OUT_Y_L_M          0x2A
#define LSM9DS1M_OUT_Y_H_M          0x2B
#define LSM9DS1M_OUT_Z_L_M          0x2C
#define LSM9DS1M_OUT_Z_H_M          0x2D
#define LSM9DS1M_INT_CFG_M          0x30
#define LSM9DS1M_INT_SRC_M          0x31
#define LSM9DS1M_INT_THS_L_M        0x32
#define LSM9DS1M_INT_THS_H_M        0x33

// Using the LSM9DS1+MS5611 Teensy 3.1 Add-On shield, ADO is set to 1 
// Seven-bit device address of accel/gyro is 110101 for ADO = 0 and 110101 for ADO = 1
#define ADO 1
#if ADO
#define LSM9DS1XG_ADDRESS 0x6B  //  Device address when ADO = 1
#define LSM9DS1M_ADDRESS  0x1E  //  Address of magnetometer
#define MS5611_ADDRESS    0x77  //  Address of altimeter
#else
#define LSM9DS1XG_ADDRESS 0x6A   //  Device address when ADO = 0
#define LSM9DS1M_ADDRESS  0x1D   //  Address of magnetometer
#define MS5611_ADDRESS    0x77   //  Address of altimeter
#endif  

//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// Use the LSM9DS1 class to create an object. [imu] can be
// named anything, we'll refer to that throught the sketch.
LSM9DS1 imu;

///////////////////////
// Example I2C Setup //
///////////////////////
// SDO_XM and SDO_G are both pulled high, so our addresses are:
// #define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
// #define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW

////////////////////////////
// Sketch Output Settings //
////////////////////////////
#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -3.4 // Declination (degrees) in Boulder, CO.

// Set initial input parameters
enum Ascale {  // set of allowable accel full scale settings
	AFS_2G = 0,
	AFS_16G,
	AFS_4G,
	AFS_8G
};

enum Aodr {  // set of allowable gyro sample rates
	AODR_PowerDown = 0,
	AODR_10Hz,
	AODR_50Hz,
	AODR_119Hz,
	AODR_238Hz,
	AODR_476Hz,
	AODR_952Hz
};

enum Abw {  // set of allowable accewl bandwidths
	ABW_408Hz = 0,
	ABW_211Hz,
	ABW_105Hz,
	ABW_50Hz
};

enum Gscale {  // set of allowable gyro full scale settings
	GFS_245DPS = 0,
	GFS_500DPS,
	GFS_NoOp,
	GFS_2000DPS
};

enum Godr {  // set of allowable gyro sample rates
	GODR_PowerDown = 0,
	GODR_14_9Hz,
	GODR_59_5Hz,
	GODR_119Hz,
	GODR_238Hz,
	GODR_476Hz,
	GODR_952Hz
};

enum Gbw {   // set of allowable gyro data bandwidths
	GBW_low = 0,  // 14 Hz at Godr = 238 Hz,  33 Hz at Godr = 952 Hz
	GBW_med,      // 29 Hz at Godr = 238 Hz,  40 Hz at Godr = 952 Hz
	GBW_high,     // 63 Hz at Godr = 238 Hz,  58 Hz at Godr = 952 Hz
	GBW_highest   // 78 Hz at Godr = 238 Hz, 100 Hz at Godr = 952 Hz
};

enum Mscale {  // set of allowable mag full scale settings
	MFS_4G = 0,
	MFS_8G,
	MFS_12G,
	MFS_16G
};

enum Mmode {
	MMode_LowPower = 0,
	MMode_MedPerformance,
	MMode_HighPerformance,
	MMode_UltraHighPerformance
};

enum Modr {  // set of allowable mag sample rates
	MODR_0_625Hz = 0,
	MODR_1_25Hz,
	MODR_2_5Hz,
	MODR_5Hz,
	MODR_10Hz,
	MODR_20Hz,
	MODR_80Hz
};


uint8_t Gscale = GFS_245DPS; // gyro full scale
uint8_t Godr = GODR_238Hz;   // gyro data sample rate
uint8_t Gbw = GBW_med;       // gyro data bandwidth
uint8_t Ascale = AFS_2G;     // accel full scale
uint8_t Aodr = AODR_238Hz;   // accel data sample rate
uint8_t Abw = ABW_50Hz;      // accel data bandwidth
uint8_t Mscale = MFS_4G;     // mag full scale
uint8_t Modr = MODR_10Hz;    // mag data sample rate
uint8_t Mmode = MMode_HighPerformance;  // magnetometer operation mode
float aRes, gRes, mRes;      // scale resolutions per LSB for the sensors

//Function definitions
void printGyro();
void printAccel();
void printMag();
void printAttitude(float ax, float ay, float az, float mx, float my, float mz);

void setup()
{

	Serial.begin(115200);

	while (!Serial) {};
	Wire1.begin();
	Wire1.setClock(400000);

	if (imu.begin(0x6B, 0X1E, Wire1) == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
	{
		Serial.print("settp");
	}

	uint32_t startMillis = millis();
	while ((millis() - 1000) < startMillis) {
		//wait
	}

	//imu.calibrate();

	startMillis = millis();
	while ((millis() - 100) < startMillis) {
		//wait
	}

	//imu.calibrateMag();
	float x[3];
	magset();

	startMillis = millis();
	while ((millis() - 100) < startMillis) {
		//wait
	}

}

void loop()
{

	// Update the sensor values whenever new data is available
	if (imu.gyroAvailable())
	{
		// To read from the gyroscope,  first call the
		// readGyro() function. When it exits, it'll update the
		// gx, gy, and gz variables with the most current data.
		imu.readGyro();
	}
	if (imu.accelAvailable())
	{
		// To read from the accelerometer, first call the
		// readAccel() function. When it exits, it'll update the
		// ax, ay, and az variables with the most current data.
		imu.readAccel();
	}

	while (!imu.magAvailable()) {};
	{
		// To read from the magnetometer, first call the
		// readMag() function. When it exits, it'll update the
		// mx, my, and mz variables with the most current data.
		imu.readMag();
	}

	if ((lastPrint + PRINT_SPEED) < millis())
	{
		printGyro();  // Print "G: gx, gy, gz"
		printAccel(); // Print "A: ax, ay, az"
		printMag();   // Print "M: mx, my, mz"
		// Print the heading and orientation for fun!
		// Call print attitude. The LSM9DS1's mag x and y
		// axes are opposite to the accelerometer, so my, mx are
		// substituted for each other.
		printAttitude(imu.ax, imu.ay, imu.az,
			-imu.my, -imu.mx, imu.mz);
		Serial.println();

		lastPrint = millis(); // Update lastPrint time
	}

}

void printGyro()
{
	// Now we can use the gx, gy, and gz variables as we please.
	// Either print them as raw ADC values, or calculated in DPS.
	Serial.print("G: ");
#ifdef PRINT_CALCULATED
	// If you want to print calculated values, you can use the
	// calcGyro helper function to convert a raw ADC value to
	// DPS. Give the function the value that you want to convert.
	Serial.print(imu.calcGyro(imu.gx), 2);
	Serial.print(", ");
	Serial.print(imu.calcGyro(imu.gy), 2);
	Serial.print(", ");
	Serial.print(imu.calcGyro(imu.gz), 2);
	Serial.println(" deg/s");
#elif defined PRINT_RAW
	Serial.print(imu.gx);
	Serial.print(", ");
	Serial.print(imu.gy);
	Serial.print(", ");
	Serial.println(imu.gz);
#endif
}

void printAccel()
{
	// Now we can use the ax, ay, and az variables as we please.
	// Either print them as raw ADC values, or calculated in g's.
	Serial.print("A: ");
#ifdef PRINT_CALCULATED
	// If you want to print calculated values, you can use the
	// calcAccel helper function to convert a raw ADC value to
	// g's. Give the function the value that you want to convert.
	Serial.print(imu.calcAccel(imu.ax), 2);
	Serial.print(", ");
	Serial.print(imu.calcAccel(imu.ay), 2);
	Serial.print(", ");
	Serial.print(imu.calcAccel(imu.az), 2);
	Serial.println(" g");
#elif defined PRINT_RAW
	Serial.print(imu.ax);
	Serial.print(", ");
	Serial.print(imu.ay);
	Serial.print(", ");
	Serial.println(imu.az);
#endif

}

void printMag()
{
	// Now we can use the mx, my, and mz variables as we please.
	// Either print them as raw ADC values, or calculated in Gauss.
	Serial.print("M: ");
#ifdef PRINT_CALCULATED
	// If you want to print calculated values, you can use the
	// calcMag helper function to convert a raw ADC value to
	// Gauss. Give the function the value that you want to convert.
	Serial.print(imu.calcMag(imu.mx), 2);
	Serial.print(", ");
	Serial.print(imu.calcMag(imu.my), 2);
	Serial.print(", ");
	Serial.print(imu.calcMag(imu.mz), 2);
	Serial.println(" gauss");
#elif defined PRINT_RAW
	Serial.print(imu.mx);
	Serial.print(", ");
	Serial.print(imu.my);
	Serial.print(", ");
	Serial.println(imu.mz);
#endif
}

// Calculate pitch, roll, and heading.
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
void printAttitude(float ax, float ay, float az, float mx, float my, float mz)
{
	float roll = atan2(ay, az);
	float pitch = atan2(-ax, sqrt(ay * ay + az * az));

	float heading;
	if (my == 0)
		heading = (mx < 0) ? PI : 0;
	else
		heading = atan2(mx, my);

	heading -= DECLINATION * PI / 180;

	if (heading > PI) heading -= (2 * PI);
	else if (heading < -PI) heading += (2 * PI);

	float Yh = (my * cos(roll)) - (mz * sin(roll));
	float Xh = (mx * cos(pitch)) + (my * sin(roll) * sin(pitch)) + (mz * cos(roll) * sin(pitch));

	float anotherYaw = atan2(Yh, Xh);

	// Convert everything from radians to degrees:
	heading *= 180.0 / PI;
	pitch *= 180.0 / PI;
	roll *= 180.0 / PI;

	Serial.print("Pitch, Roll: ");
	Serial.print(pitch, 2);
	Serial.print(", ");
	Serial.println(roll, 2);
	Serial.print("Heading: "); Serial.println(heading, 2);
	Serial.print("Yaw: "); Serial.println(((heading = atan2(mx, my)) * 180.0 / PI), 2);
	Serial.print("Anothther Yaw: "); Serial.println(anotherYaw * 180.0 / PI, 2);
}

void magset() {

	mRes = 4.0 / 32768.0;

	uint8_t data[6]; // data array to hold mag x, y, z, data
	uint16_t ii = 0, sample_count = 0;
	int32_t mag_bias[3] = { 0, 0, 0 };
	int16_t mag_max[3] = { 0, 0, 0 }, mag_min[3] = { 0, 0, 0 };

	// configure the magnetometer-enable temperature compensation of mag data
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG1_M, 0x80 | 2 << 5 | 4 << 2); // select x,y-axis mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG2_M, 0 << 5); // select mag full scale
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG3_M, 0x00); // continuous conversion mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG4_M, 2 << 2); // select z-axis mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG5_M, 0x40); // select block update mode

	mag_bias[0] = (501.46 / 1000) / mRes;
	mag_bias[1] = (79.22 / 1000) / mRes;
	mag_bias[2] = (15.75 / 1000) / mRes;

	//write biases to accelerometermagnetometer offset registers as counts);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_X_REG_L_M, (int16_t)mag_bias[0] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_X_REG_H_M, ((int16_t)mag_bias[0] >> 8) & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Y_REG_L_M, (int16_t)mag_bias[1] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Y_REG_H_M, ((int16_t)mag_bias[1] >> 8) & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Z_REG_L_M, (int16_t)mag_bias[2] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Z_REG_H_M, ((int16_t)mag_bias[2] >> 8) & 0xFF);

	Serial.println("Mag Calibration done!");
};


void magcalLSM9DS1(float* dest1)
{
	uint8_t data[6]; // data array to hold mag x, y, z, data
	uint16_t ii = 0, sample_count = 0;
	int32_t mag_bias[3] = { 0, 0, 0 };
	int16_t mag_max[3] = { 0, 0, 0 }, mag_min[3] = { 0, 0, 0 };

	// configure the magnetometer-enable temperature compensation of mag data
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG1_M, 0x80 | Mmode << 5 | Modr << 2); // select x,y-axis mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG2_M, Mscale << 5); // select mag full scale
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG3_M, 0x00); // continuous conversion mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG4_M, Mmode << 2); // select z-axis mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG5_M, 0x40); // select block update mode

	Serial.println("Mag Calibration: Wave device in a figure eight until done!");
	delay(4000);

	sample_count = 128;
	//sample_count = 16;
	for (ii = 0; ii < sample_count; ii++) {
		int16_t mag_temp[3] = { 0, 0, 0 };
		readBytes(LSM9DS1M_ADDRESS, LSM9DS1M_OUT_X_L_M, 6, &data[0]);  // Read the six raw data registers into data array
		mag_temp[0] = (int16_t)(((int16_t)data[1] << 8) | data[0]);   // Form signed 16-bit integer for each sample in FIFO
		mag_temp[1] = (int16_t)(((int16_t)data[3] << 8) | data[2]);
		mag_temp[2] = (int16_t)(((int16_t)data[5] << 8) | data[4]);
		for (int jj = 0; jj < 3; jj++) {
			if (mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
			if (mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
		}
		delay(105);  // at 10 Hz ODR, new mag data is available every 100 ms
	}

	//    Serial.println("mag x min/max:"); Serial.println(mag_max[0]); Serial.println(mag_min[0]);
	//    Serial.println("mag y min/max:"); Serial.println(mag_max[1]); Serial.println(mag_min[1]);
	//    Serial.println("mag z min/max:"); Serial.println(mag_max[2]); Serial.println(mag_min[2]);

	mag_bias[0] = (mag_max[0] + mag_min[0]) / 2;  // get average x mag bias in counts
	mag_bias[1] = (mag_max[1] + mag_min[1]) / 2;  // get average y mag bias in counts
	mag_bias[2] = (mag_max[2] + mag_min[2]) / 2;  // get average z mag bias in counts

	dest1[0] = (float)mag_bias[0] * mRes;  // save mag biases in G for main program
	dest1[1] = (float)mag_bias[1] * mRes;
	dest1[2] = (float)mag_bias[2] * mRes;

	mag_bias[0] = (501.46 / 1000) / mRes;
	mag_bias[1] = (79.22 / 1000) / mRes;
	mag_bias[2] = (15.75 / 1000) / mRes;

	//write biases to accelerometermagnetometer offset registers as counts);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_X_REG_L_M, (int16_t)mag_bias[0] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_X_REG_H_M, ((int16_t)mag_bias[0] >> 8) & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Y_REG_L_M, (int16_t)mag_bias[1] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Y_REG_H_M, ((int16_t)mag_bias[1] >> 8) & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Z_REG_L_M, (int16_t)mag_bias[2] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Z_REG_H_M, ((int16_t)mag_bias[2] >> 8) & 0xFF);

	Serial.println("Mag Calibration done!");
}

void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t* dest)
{
	Wire1.beginTransmission(address);   // Initialize the Tx buffer
	Wire1.write(subAddress);            // Put slave register address in Tx buffer
	//  Wire.endTransmission(I2C_NOSTOP);  // Send the Tx buffer, but send a restart to keep connection alive
	Wire1.endTransmission(false);       // Send the Tx buffer, but send a restart to keep connection alive
	uint8_t i = 0;
	Wire1.requestFrom(address, count);  // Read bytes from slave register address 
	//        Wire.requestFrom(address, (size_t) count);  // Read bytes from slave register address 
	while (Wire1.available()) {
		dest[i++] = Wire1.read();
	}         // Put read results in the Rx buffer
}

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire1.beginTransmission(address);  // Initialize the Tx buffer
	Wire1.write(subAddress);           // Put slave register address in Tx buffer
	Wire1.write(data);                 // Put data in Tx buffer
	Wire1.endTransmission();           // Send the Tx buffer
}