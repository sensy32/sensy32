/* 
  BME280 Sensor Data Transmission Example with Sensy Board
  This code reads data from BME280 sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.
*/

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h> 
#include <HTTPClient.h>
#include <ss_oled.h> 
#include "SparkFunBME280.h"

//Board configuration
String apiKey = "board-api-key"; // Change here your Board API key 

// Web Server Configuration
const char *ssid = "wifi-name"; // Change here your Wi-Fi network SSID (name) 
const char* password = "wifi-password"; // Change here your Wi-Fi network password 
const char *server = "https://sensy32.io";
const int port = 443;

//Sensor Configuration
BME280 mySensor; 

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
  Serial.println("Reading basic values from BME280");

  Wire.begin(7, 6);
  setupWiFi();
  mySensor.setI2CAddress(0x76);
  if (mySensor.beginI2C(Wire) == false)  //Begin communication over I2C
  {
    Serial.println("The sensor did not respond. Please check wiring.");
    while (1)
      ;  //Freeze
  }

setupLcd();
}

void readAndSendSensorData() {
  //float temperature = mySensor.readTempF(); 
  float temperature = mySensor.readTempC();
  float humidity = mySensor.readFloatHumidity();
  float pressure = mySensor.readFloatPressure();
  float altitude = mySensor.readFloatAltitudeFeet();
  if (isnan(temperature) || isnan(humidity) || isnan(pressure) || isnan(altitude)) {
    Serial.println("Error reading sensor data");
    return;
  }
  Serial.println("Humidity: ");
  Serial.print(humidity, 0);

  Serial.println("Pressure: ");
  Serial.print(pressure, 0);

  Serial.println(" Altitude: ");
  //Serial.print(mySensor.readFloatAltitudeMeters(), 1);
  Serial.print(altitude, 1);

  Serial.println(" Temperature: ");
  //Serial.print(mySensor.readTempC(), 2);
  Serial.print(temperature, 2);

  // Send Data to SENSY32.io
  sendDataToSensy(temperature, humidity, pressure, altitude); 
  // Send Data to LCD
  sendDataToLcd(temperature,humidity, pressure, altitude);

}

void sendDataToSensy(float temperature, float humidity, float pressure, float altitude) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.connect(server, port);

    HTTPClient http;

    // Append query parameters to the URL
    String url = String(server) + "/sensors/api/data?Temperature=" + temperature + "&Humidity=" + humidity + "&Pressure=" + pressure + "&Altitude=" + altitude;
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

void sendDataToLcd(float temperature, float humidity, float pressure, float altitude) {
  int i, x, y;
  char szTemp[32];
  unsigned long ms; 

  // Display on OLED
  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled, 0, 16, 0, (char *)"Temperature: ", FONT_NORMAL, 0, 1);
  dtostrf(temperature, 10, 2, szTemp);  // Format: %10.2f (10 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1);

  oledWriteString(&ssoled, 0, 16, 4, (char *)"Humidity: ", FONT_NORMAL, 0, 1);
  dtostrf(humidity, 10,2 , szTemp); 
  oledWriteString(&ssoled, 0, 0, 6, szTemp, FONT_NORMAL, 0, 1);
  delay(2000);

  oledWriteString(&ssoled, 0, 16, 0, (char *)"Pressure: ", FONT_NORMAL, 0, 1);
  dtostrf(pressure, 10, 2, szTemp); 
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1);

  oledWriteString(&ssoled, 0, 16, 4, (char *)"Altitude: ", FONT_NORMAL, 0, 1);
  dtostrf(altitude, 10,2 , szTemp);  
  oledWriteString(&ssoled, 0, 0, 6, szTemp, FONT_NORMAL, 0, 1); 

  delay(2000);
}

void loop() {
  readAndSendSensorData();
  delay(50);
}
