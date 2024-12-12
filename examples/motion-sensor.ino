/* 
  STHS34PF80 Sensor Data Transmission Example with Sensy Board
  This code reads data from STHS34PF80 sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.
*/

#include "SparkFun_STHS34PF80_Arduino_Library.h"
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <HTTPClient.h>
#include <ss_oled.h> 

//Board configuration
String apiKey = "board-api-key"; // Change here your Board API key 

// Web Server Configuration
const char *ssid = "wifi-name"; // Change here your Wi-Fi network SSID (name) 
const char* password = "wifi-password"; // Change here your Wi-Fi network password 
const char *server = "https://sensy32.io";
const int port = 443;

// Sensor Configuration
STHS34PF80_I2C mySensor;

// Global Presence Value
int16_t presenceVal = 0; 

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
  Serial.println("STHS34PF80 Example 5: Arduino Serial Plotter Output");
  setupWiFi();

  // Begin I2C
  if (Wire.begin(7, 6) == 0) {
    Serial.println("I2C Error - check I2C Address");
    while (1)
      ;
  }

  // Establish communication with device
  if (mySensor.begin() != 0) {
    Serial.println("Error setting up device - please check wiring.");
    while (1)
      ;
  }
  setupLcd();

  Serial.println("Open the Serial Plotter for graphical viewing");

  delay(1000);
}
void sendDataToLcd(float presenceVal) {
  int i, x, y;
  char szTemp[32];
  unsigned long ms;
  // Display on OLED
  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled, 0, 16, 0, (char *)"presenceVal: ", FONT_NORMAL, 0, 1);
  dtostrf(presenceVal, 4, 2, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1);
  delay(1000);
}
void sendDataToSensy(float presenceVal) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.connect(server, port);

    HTTPClient http;

    // Append query parameters to the URL
    String url = String(server) + "/sensors/api/data?presenceVal=" + presenceVal;

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
void loop() {
  sths34pf80_tmos_drdy_status_t dataReady;
  mySensor.getDataReady(&dataReady);

  // Check whether sensor has new data - run through loop if data is ready
  // if (dataReady.drdy == 1) {
  //   sths34pf80_tmos_func_status_t status;
  //   mySensor.getStatus(&status);
    // If the flag is high, then read out the information
  //   if (status.pres_flag == 1) {
  mySensor.getPresenceValue(&presenceVal);
  sendDataToLcd(presenceVal);
  sendDataToSensy(presenceVal);
  //   }
  // }
}
