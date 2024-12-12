/*
  BNO08x Sensor Data Transmission Example with Sensy Board
  This code reads accelerometer data from the BNO08x sensor, displays it on an OLED screen, and sends it to a server via WiFi to (https://sensy32.io)
*/

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <HTTPClient.h>
#include <ss_oled.h>
#include "SparkFun_BNO08x_Arduino_Library.h"  // http://librarymanager/All#SparkFun_BNO08x

//Board configuration
String apiKey = "board-api-key"; // Change here your Board API key 

// Web Server Configuration
const char *ssid = "wifi-name"; // Change here your Wi-Fi network SSID (name) 
const char* password = "wifi-password"; // Change here your Wi-Fi network password 
const char *server = "https://sensy32.io";
const int port = 443;

//Sensor Configuration
BNO08x myIMU;

//OLED / LCD Configuration

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
//#define MY_OLED OLED_64x32
//#define OLED_WIDTH 64
//#define OLED_HEIGHT 32

SSOLED ssoled;

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
  Serial.println("BNO08x Read Example");

  Wire.begin(7, 6);

  if (myIMU.begin() == false) {
    Serial.println("BNO08x not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1)
      ;
  }
  Serial.println("BNO08x found!");

  // Wire.setClock(400000); //Increase I2C data rate to 400kHz
  setupLcd();
  // setReports();

  Serial.println("Reading events");
  delay(100);
}

void sendDataToLcd(float valX, float valY, float valZ) {
  int i, x, y;
  char szTemp[32];
  unsigned long ms;
  // Display on OLED
  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled, 0, 16, 0, (char *)"X: ", FONT_NORMAL, 0, 1);
  dtostrf(valX, 2, 0, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 32, 0, szTemp, FONT_NORMAL, 0, 1);

  oledWriteString(&ssoled, 0, 16, 2, (char *)"Y: ", FONT_NORMAL, 0, 1);
  dtostrf(valY, 2, 0, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 32, 2, szTemp, FONT_NORMAL, 0, 1);

  oledWriteString(&ssoled, 0, 16, 4, (char *)"Z: ", FONT_NORMAL, 0, 1);
  dtostrf(valZ, 2, 0, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 32, 4, szTemp, FONT_NORMAL, 0, 1);
  delay(2000);
}

void sendDataToSensy(float x, float y, float z) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.connect(server, port);

    HTTPClient http;

    // Append query parameters to the URL
    String url = String(server) + "/sensors/api/data?X=" + x + "&Y=" + y + "&Z=" + z;

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"api_key\":\"" + apiKey + "\"}";
    Serial.println(jsonBody);
    int httpCode = http.POST(jsonBody);     
    Serial.print("HTTP result: ");
    Serial.println(httpCode);

    // Print the response to the serial monitor
    http.writeToStream(&Serial);

    // End the HTTP request
    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }
}
// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (myIMU.enableAccelerometer() == true) {
    Serial.println(F("Accelerometer enabled"));
    Serial.println(F("Output in form x, y, z, in m/s^2"));
  } else {
    Serial.println("Could not enable accelerometer");
  }
}

void loop() {
  delay(10);

  if (myIMU.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  // Has a new event come in on the Sensor Hub Bus?
  if (myIMU.getSensorEvent() == true) {

    // is it the correct sensor data we want?
    if (myIMU.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {

      float x = myIMU.getAccelX();
      float y = myIMU.getAccelY();
      float z = myIMU.getAccelZ();

      Serial.print(x, 2);
      Serial.print(F(","));
      Serial.print(y, 2);
      Serial.print(F(","));
      Serial.print(z, 2);
      sendDataToLcd(x, y, z);
      sendDataToSensy(x, y, z);
      Serial.println();
    }
  }
}
