/* 
  UV Sensor Data Transmission Example with Sensy Board
  This code reads UV data from the Adafruit LTR390 sensor, displays it on an OLED screen, and sends it to a server via WiFi to (https://sensy32.io)
*/
#include "config.h" // config file shared between all files
#include "Adafruit_LTR390.h"

// Sensor 
Adafruit_LTR390 ltr = Adafruit_LTR390();

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
  setupWiFi();  //setup wifi
  Serial.println("Adafruit LTR-390 test");

  Wire.begin(7, 6);
  if (!ltr.begin()) {
    Serial.println("Couldn't find LTR sensor!");
    while (1);
  }
  Serial.println("Found LTR sensor!");

  setupLcd();
  ltr.setMode(LTR390_MODE_UVS);
  if (ltr.getMode() == LTR390_MODE_ALS) {
    Serial.println("In ALS mode");
  } else {
    Serial.println("In UVS mode");
  }

  ltr.setGain(LTR390_GAIN_3);
  Serial.print("Gain : ");
  switch (ltr.getGain()) {
    case LTR390_GAIN_1: Serial.println(1); break;
    case LTR390_GAIN_3: Serial.println(3); break;
    case LTR390_GAIN_6: Serial.println(6); break;
    case LTR390_GAIN_9: Serial.println(9); break;
    case LTR390_GAIN_18: Serial.println(18); break;
  }

  ltr.setResolution(LTR390_RESOLUTION_16BIT);
  Serial.print("Resolution : ");
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

void sendDataToSensy(float uv) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.connect(server, port);

    HTTPClient http;

    String url = String(server) + "/sensors/api/data?apiKey=" + apiKey;

    JsonDocument jsonBody;
    String jsonBodyString;

    jsonBody["uv"] = "{\"UV\":\"" + String(uv) + "\"}";

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
void sendDataToLcd(float uv) {
  int i, x, y;
  char szTemp[32];
  unsigned long ms;
  // Display on OLED
  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled, 0, 16, 0, (char *)"UV: ", FONT_NORMAL, 0, 1);
  dtostrf(uv, 4, 2, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1); 
  delay(2000);
}

void loop() {
  // if (ltr.newDataAvailable()) {
  Serial.print("UV data: ");
  float uvData = ltr.readUVS();
  Serial.println(uvData);
  sendDataToLcd(uvData);
  sendDataToSensy(uvData);
  //}
  delay(50);
}
