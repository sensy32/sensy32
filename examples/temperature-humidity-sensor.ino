/* 
  BME280 Sensor Data Transmission Example with Sensy Board
  This code reads data from BME280 sensor and sends it to a web server (https://sensy32.io) and displays it on an OLED screen.
*/
#include "config.h" // config file shared between all files
#include <Wire.h> 
#include "SparkFunBME280.h"

//Sensor Configuration
BME280 mySensor; 

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
