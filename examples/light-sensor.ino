/* 
  TSL2591 Digital Light Sensor Example with Sensy Board
  This code reads data from TSL2591 light sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.
  Dynamic Range: 600M:1 
  Maximum Lux: 88K 
*/

#include "config.h" // config file shared between all files
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

//Sensor configuration
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);  // pass in a number for the sensor identifier (for your use later) 

SSOLED ssoled;

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

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2591
*/
/**************************************************************************/
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

void setupLcd(){
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
  Serial.println(F("Starting Adafruit TSL2591 Test!"));
  Wire.begin(7, 6);
  if (tsl.begin()) {
    Serial.println(F("Found a TSL2591 sensor"));
  } else {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }

  setupLcd();

/* Display some basic information on this sensor */
displaySensorDetails();

/* Configure the sensor */
configureSensor();

// Now we're ready to get readings ... move on to loop()!
}

void simpleRead(void) {
  // Simple data read example. Just read the infrared, fullspecrtrum diode
  // or 'visible' (difference between the two) channels.
  // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  //uint16_t x = tsl.getLuminosity(TSL2591_FULLSPECTRUM);
  //uint16_t x = tsl.getLuminosity(TSL2591_INFRARED);

  Serial.print(F("[ "));
  Serial.print(millis());
  Serial.print(F(" ms ] "));
  Serial.print(F("Luminosity: "));
  Serial.println(x, DEC);
}

/**************************************************************************/
/*
    Show how to read IR and Full Spectrum at once and convert to lux
*/
/**************************************************************************/
void advancedRead(void) {
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
  float lux = tsl.calculateLux(full, ir);
  sendDataToLcd(ir, full, full - ir, lux);
  sendDataToSensy(ir, full, full - ir, lux);
}

void unifiedSensorAPIRead(void) {
  /* Get a new sensor event */
  sensors_event_t event;
  tsl.getEvent(&event);

  /* Display the results (light is measured in lux) */
  Serial.print(F("[ "));
  Serial.print(event.timestamp);
  Serial.print(F(" ms ] "));
  if ((event.light == 0) | (event.light > 4294966000.0) | (event.light < -4294966000.0)) {
    /* If event.light = 0 lux the sensor is probably saturated */
    /* and no reliable data could be generated! */
    /* if event.light is +/- 4294967040 there was a float over/underflow */
    Serial.println(F("Invalid data (adjust gain or timing)"));
  } else {
    Serial.print(event.light);
    Serial.println(F(" lux"));
  }
}


void sendDataToSensy(float ir, float full, float visible, float lux) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.connect(server, port);

    HTTPClient http;

    JsonDocument jsonBody;
    String jsonBodyString;

    String url = String(server) + "/sensors/api/data?apiKey=" + apiKey;

    jsonBody["light"] = "{\"IR\":\"" + String(ir) + "\",\"Full\":\"" + String(full) + "\",\"Visible\":\"" + String(visible) + "\",\"Lux\":\"" + String(lux) + "\"}";

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
void sendDataToLcd(float ir, float full, float visible, float lux) {
  int i, x, y;
  char szTemp[32];
  unsigned long ms;

  // Display on OLED
  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled, 0, 16, 0, (char *)"IR: ", FONT_NORMAL, 0, 1);
  dtostrf(ir, 10, 2, szTemp);  // Format: %10.2f (10 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1);

  oledWriteString(&ssoled, 0, 16, 4, (char *)"Full: ", FONT_NORMAL, 0, 1);
  dtostrf(full, 10, 2, szTemp);  
  oledWriteString(&ssoled, 0, 0, 6, szTemp, FONT_NORMAL, 0, 1);
  delay(2000);

  oledWriteString(&ssoled, 0, 16, 0, (char *)"Visible: ", FONT_NORMAL, 0, 1);
  dtostrf(visible, 10, 2, szTemp); 
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1);

  oledWriteString(&ssoled, 0, 16, 4, (char *)"Lux: ", FONT_NORMAL, 0, 1);
  dtostrf(lux, 10, 2, szTemp); 
  oledWriteString(&ssoled, 0, 0, 6, szTemp, FONT_NORMAL, 0, 1);

  delay(2000);
}

void loop(void) {
  //simpleRead();
  advancedRead();
  // unifiedSensorAPIRead(); 
  delay(500);
}
