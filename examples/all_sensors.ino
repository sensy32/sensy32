/* 
  BMP388 Sensor Data Transmission Example with Sensy Board 
  This code reads temperature, pressure, and altitude data from the BMP388 sensor, displays it on an OLED screen, and sends it to a server via WiFi to (https://sensy32.io)
  
  UV Sensor Data Transmission Example with Sensy Board
  This code reads UV data from the Adafruit LTR390 sensor, displays it on an OLED screen, and sends it to a server via WiFi to (https://sensy32.io)

  BME280 Sensor Data Transmission Example with Sensy Board 
  This code reads data from BME280 sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.

  BNO08x Sensor Data Transmission Example with Sensy Board
  This code reads data from BNO08x sensor and sends it to a web server and displays it on an OLED screen.

  STHS34PF80 Sensor Data Transmission Example with Sensy Board
  This code reads data from STHS34PF80 sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.

  TSL2591 Digital Light Sensor Example with Sensy Board
  This code reads data from TSL2591 light sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.
  Dynamic Range: 600M:1 
  Maximum Lux: 88K 

  BNO08x Sensor Data Transmission Example with Sensy Board
  This code reads accelerometer data from the BNO08x sensor, displays it on an OLED screen, and sends it to a server via WiFi to (https://sensy32.io)
*/

/*
  Packages needed for all Sensors 
*/
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <HTTPClient.h>
#include <ss_oled.h>

#include <Wire.h> // package needed for motion, accelerometer, and light Sensors 

/*
  Specific packages for each Sensor 
*/
#include "SparkFun_BNO08x_Arduino_Library.h" // package needed for Accelerometer and Orientation Sensors // http://librarymanager/All#SparkFun_BNO08x
#include <Adafruit_Sensor.h> // package needed for Light Sensor
#include "Adafruit_TSL2591.h" // package needed for Light Sensor
#include "SparkFun_STHS34PF80_Arduino_Library.h" // package needed for Motion Sensor
#include <BMP388_DEV.h> // package needed for Altitude - Pressure Sensor
#include "SparkFunBME280.h"  // package needed for Temperature - Humidity Sensor
#include "Adafruit_LTR390.h" // package needed for UV Sensor

// Board configuration
String apiKey = "1a1aaef909172cbe0d1bc43441b0b4f290490573"; // Change here your Board API key 

// Web Server Configuration
const char *ssid = "OLINDIAS"; // Change here your Wi-Fi network SSID (name) 
const char* password = "BABAvv.2700"; // Change here your Wi-Fi network password 
const char *server = "https://sensy32.io";
const int port = 443;

// Sensors Configurations
BNO08x myIMUAcc; // accelerometer
BNO08x myIMUOr; // orientation
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // light // pass in a number for the sensor identifier (for your use later) 
STHS34PF80_I2C mySensorMo; // motion
BMP388_DEV bmp388; // pressure and altitude
BME280 mySensorTem;  // temperature and humidity
Adafruit_LTR390 ltr = Adafruit_LTR390(); // uv

/*
  OLED / LCD Configuration
*/

// if your system doesn't have enough RAM for a back buffer, comment out
// this line (e.g. ATtiny85)
#define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

// Use -1 for the Wire library default pins
// or specify the pin numbers to use with the Wire library or bit banging on any GPIO pins
// These are the pin numbers for the M5Stack Atom default I2C
#define SDA_PIN -1
#define SCL_PIN -1
// Set this to -1 to disable or the GPIO pin number connected to the reset
// line of your display if it requires an external reset
#define RESET_PIN -1
// let ss_oled figure out the display address
#define OLED_ADDR -1
// don't rotate the display
#define FLIP180 0
// don't invert the display
#define INVERT 0
// Bit-Bang the I2C bus
#define USE_HW_I2C 1

// Change these if you're using a different OLED display
#define MY_OLED OLED_128x64
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

SSOLED ssoled;

// For reliable interaction with the SHTP bus, we need
// to use hardware reset control, and monitor the H_INT pin
// The H_INT pin will go low when its okay to talk on the SHTP bus.
// Note, these can be other GPIO if you like.
// Do not define (or set to -1) to not user these features.
#define BNO08X_INT 34
//#define BNO08X_INT  -1
#define BNO08X_RST 35
//#define BNO08X_RST  -1

unsigned long lastMillis = 0;  // Keep track of time
bool lastPowerState = true;    // Toggle between "On" and "Sleep"

#define BNO08X_ADDR 0x4B  // SparkFun BNO08x Breakout (Qwiic) defaults to 0x4B
//#define BNO08X_ADDR 0x4A // Alternate address if ADR jumper is closed 

// Global Variable Definitions
int16_t presenceVal = 0; // presence sensor
volatile boolean dataReady = false; // pressure and altitude sensor
float temperature, pressure, altitude; // pressure and altitude sensor
int int_pin = 33; // pressure and altitude sensor

void setupWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
}

void setupLcd() {
  //LCD Setup
  int rc;
  // The I2C SDA/SCL pins set to -1 means to use the default Wire library 
  // If pins were specified, they would be bit-banged in software
  // This isn't inferior to hw I2C and in fact allows you to go faster on certain CPUs
  // The reset pin is optional and I've only seen it needed on larger OLEDs (2.4")
  //    that can be configured as either SPI or I2C
  //
  // oledInit(SSOLED *, type, oled_addr, rotate180, invert, bWire, SDA_PIN, SCL_PIN, RESET_PIN, speed)

  rc = oledInit(&ssoled, MY_OLED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L);  // use standard I2C bus at 400Khz
  if (rc != OLED_NOT_FOUND) {
    char *msgs[] = { (char *)"SSD1306 @ 0x3C", (char *)"SSD1306 @ 0x3D", (char *)"SH1106 @ 0x3C", (char *)"SH1106 @ 0x3D" };
    oledFill(&ssoled, 0, 1);
    oledWriteString(&ssoled, 0, 0, 0, msgs[rc], FONT_NORMAL, 0, 1);
    oledSetBackBuffer(&ssoled, ucBackBuffer);
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  Serial.println();
  Wire.begin(7, 6);

  // light 
  if(tsl.begin()==false){
    Serial.println("No sensor for Light found ... check your wiring?");
    while (1);
  } else if(tsl.begin()==true){
    Serial.println("Found a TSL2591 for Light sensor.");
    displaySensorDetails();
    configureSensor();
  }

  // motion
  if(mySensorMo.begin()==false){
    Serial.println("No sensor for Motion found ... check your wiring?");
    while (1);
  } else if(mySensorMo.begin()==true){
    Serial.println("Found a STHS34PF80_I2C for Motion sensor.");
    Serial.println("Open the Serial Plotter for graphical viewing");
    delay(1000);
  }

  // pressure and altitude
  if(bmp388.begin()==false){
    Serial.println("No BMP388 Sensor found for Pressure and Altitude.");
    while (1); 
  }else if(bmp388.begin()==true){ // Default initialisation, place the BMP388 into SLEEP_MODE
    bmp388.enableInterrupt();                                                   // Enable the BMP388's interrupt (INT) pin
    attachInterrupt(digitalPinToInterrupt(int_pin), interruptHandler, RISING);  // Set interrupt to call interruptHandler function on D2
    bmp388.setTimeStandby(TIME_STANDBY_1280MS);                                 // Set the standby time to 1.3 seconds
    bmp388.startNormalConversion();                                             // Start BMP388 continuous conversion in NORMAL_MODE
    Serial.println("BMP388 Sensor found for Pressure and Altitude.");
  }

  // temperature and humidity
  mySensorTem.setI2CAddress(0x76);
  if (mySensorTem.beginI2C(Wire)==false){ //Begin communication over I2C
    Serial.println("No BMP388 Sensor found for Temperature and Humidity.");
    while (1);  //Freeze
  }else if(mySensorTem.beginI2C(Wire)==true){
    Serial.println("BMP388 Sensor found for Temperature and Humidity.");
  }

  // uv 
  if (ltr.begin()==false) {
    Serial.println("Couldn't find LTR sensor for UV!");
    while (1);
  }else if(ltr.begin()==true){

    Serial.println("Found a LTR sensor for UV!");
    ltr.setMode(LTR390_MODE_UVS);
    if (ltr.getMode() == LTR390_MODE_ALS) {
      Serial.println("LTR sensor is in ALS mode");
    } else {
      Serial.println("LTR sensor is in UVS mode");
    }

    ltr.setGain(LTR390_GAIN_3);
    Serial.println("LTR sensor Gain : ");
    switch (ltr.getGain()) {
      case LTR390_GAIN_1: Serial.println(1); break;
      case LTR390_GAIN_3: Serial.println(3); break;
      case LTR390_GAIN_6: Serial.println(6); break;
      case LTR390_GAIN_9: Serial.println(9); break;
      case LTR390_GAIN_18: Serial.println(18); break;
    }

    ltr.setResolution(LTR390_RESOLUTION_16BIT);
    Serial.println("LTR sensor Resolution : ");
    switch (ltr.getResolution()) {
      case LTR390_RESOLUTION_13BIT: Serial.println(13); break;
      case LTR390_RESOLUTION_16BIT: Serial.println(16); break;
      case LTR390_RESOLUTION_17BIT: Serial.println(17); break;
      case LTR390_RESOLUTION_18BIT: Serial.println(18); break;
      case LTR390_RESOLUTION_19BIT: Serial.println(19); break;
      case LTR390_RESOLUTION_20BIT: Serial.println(20); break;
    }

    ltr.setThresholds(100, 1000);
    ltr.configInterrupt(true, LTR390_MODE_UVS);
  }

  setupLcd();
}

void displaySensorDetails(void) {
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print(F("Sensor:       "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:   "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:    "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:    "));
  Serial.print(sensor.max_value);
  Serial.println(F(" lux"));
  Serial.print(F("Min Value:    "));
  Serial.print(sensor.min_value);
  Serial.println(F(" lux"));
  Serial.print(F("Resolution:   "));
  Serial.print(sensor.resolution, 4);
  Serial.println(F(" lux"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}

void interruptHandler() { // Interrupt handler function
  dataReady = true;  // Set the dataReady flag
}

void configureSensor(void) {
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);  // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */
  Serial.println(F("------------------------------------"));
  Serial.print(F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch (gain) {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print(F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

void getLight(void){
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print(F("[ "));
  Serial.print(millis());
  Serial.print(F(" ms ] "));
  Serial.print(F("IR: "));
  Serial.print(ir);
  Serial.print(F("  "));
  Serial.print(F("Full: "));
  Serial.print(full);
  Serial.print(F("  "));
  Serial.print(F("Visible: "));
  Serial.print(full - ir);
  Serial.print(F("  "));
  Serial.print(F("Lux: "));
  Serial.println(tsl.calculateLux(full, ir), 6);
  // float lux = tsl.calculateLux(full, ir);
}

void getPressureAltitude(void) {
  if (dataReady) {
    bmp388.getMeasurements(temperature, pressure, altitude);  // Read the measurements
    Serial.print(temperature);                                // Display the results
    Serial.print(F("*C   "));
    Serial.print(pressure);
    Serial.print(F("hPa   "));
    Serial.print(altitude);
    Serial.println(F("m"));
    dataReady = false;  // Clear the dataReady flag
  }
}

void getTemperatureHumidityData(void){
  //float temperature = mySensorTem.readTempF(); 
  float temperature = mySensorTem.readTempC();
  float humidity = mySensorTem.readFloatHumidity();
  float pressure = mySensorTem.readFloatPressure();
  float altitude = mySensorTem.readFloatAltitudeFeet();
  if (isnan(temperature) || isnan(humidity) || isnan(pressure) || isnan(altitude)) {
    Serial.println("Error reading sensor data");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(humidity, 0);
  Serial.print(F("g.m-3   "));

  Serial.print(" Pressure: ");
  Serial.print(pressure, 0);
  Serial.print(F("Pa   "));

  Serial.print(" Altitude: ");
  Serial.print(altitude, 1);
  Serial.print(F("ft   "));

  Serial.print(" Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(F("*C   "));

}

void getUvSensorData(void){
  Serial.print("UV data: ");
  float uvData = ltr.readUVS();
  Serial.println(uvData);
}

void loop() {

  // light sensor
  Serial.println(F("------------------------------------"));
  getLight();
  
  // altitude and pressure sensor
  Serial.println(F("------------------------------------"));
  getPressureAltitude();

  // temperature and humidity sensor
  Serial.println(F("------------------------------------"));
  getTemperatureHumidityData();

  // uv sensor
  Serial.println(F("------------------------------------"));
  getUvSensorData();

  delay(50);
}
