/* 
  STHS34PF80 Sensor Data Transmission Example with Sensy Board
  This code reads data from STHS34PF80 sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.
*/

#include "SparkFun_STHS34PF80_Arduino_Library.h"
#include <Wire.h>
#include "config.h" // config file shared between all files

// Sensor Configuration
STHS34PF80_I2C mySensor;

// Global Presence Value
int16_t presenceVal = 0; 

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

  Wire.begin(7, 6);

  // Establish communication with device
  if (mySensor.begin() == false) {
    Serial.println("Error setting up device - please check wiring.");
    while (1);
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

    String url = String(server) + "/sensors/api/data?apiKey=" + apiKey;

    JsonDocument jsonBody;
    String jsonBodyString;

    jsonBody["motion"] = "{\"presenceVal\":\"" + String(presenceVal) + "\"}";

    serializeJson(jsonBody, jsonBodyString);

    Serial.println();
    Serial.println(url);
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    Serial.println(jsonBodyString);
    int httpCode = http.POST(jsonBodyString);     
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
