/* 
  BNO08x Sensor Data Transmission Example with Sensy Board
  This code reads data from BNO08x sensor and sends it to a web server and displays it on an OLED screen.
*/
#include "config.h" // config file shared between all files
#include "SparkFun_BNO08x_Arduino_Library.h" // http://librarymanager/All#SparkFun_BNO08x

//Sensor Configuration
BNO08x myIMU;

unsigned long lastMillis = 0;  // Keep track of time
bool lastPowerState = true;    // Toggle between "On" and "Sleep"

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
  Serial.println("BNO08x Sleep Example");

  //Wire.begin();
  Wire.begin(7, 6);
  if (myIMU.begin() == false) {
    Serial.println("BNO08x not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }
  
  Serial.println("BNO08x found!");
  setupLcd();
  setReports();

  Serial.println("Reading events");
  delay(100);

  lastMillis = millis();  // Keep track of time
}

void sendDataToSensy(float quatI, float quatJ, float quatK, float quatReal, float accuracy) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure();
    client.connect(server, port);

    HTTPClient http;

    String url = String(server) + "/sensors/api/data?apiKey=" + apiKey;

    JsonDocument jsonBody;
    String jsonBodyString;

    jsonBody["orientation"] = "{\"quatI\":\"" + String(quatI) + "\",\"quatJ\":\"" + String(quatJ) + "\",\"quatK\":\"" + String(quatK) + "\",\"quatReal\":\"" + String(quatReal) + "\",\"quatRadianAccuracy\":\"" + String(accuracy) + "\"}";

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

void sendDataToLcd(float quatI, float quatJ, float quatK, float quatReal, float accuracy) {
  int i, x, y;
  char szTemp[32];
  unsigned long ms;
  // Display on OLED
  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled, 0, 16, 0, (char *)"quatI: ", FONT_NORMAL, 0, 1);
  dtostrf(quatI, 4, 2, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1); 

  oledWriteString(&ssoled, 0, 16, 4, (char *)"quatJ: ", FONT_NORMAL, 0, 1);
  dtostrf(quatJ, 4, 2, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 6, szTemp, FONT_NORMAL, 0, 1); 

  delay(2000);
   oledWriteString(&ssoled, 0, 16, 0, (char *)"quatK: ", FONT_NORMAL, 0, 1);
  dtostrf(quatK, 4, 2, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 2, szTemp, FONT_NORMAL, 0, 1); 

  oledWriteString(&ssoled, 0, 16, 4, (char *)"quatReal: ", FONT_NORMAL, 0, 1);
  dtostrf(quatReal, 4, 2, szTemp);  // Format: %4.2f (4 digits total, 2 after decimal point)
  oledWriteString(&ssoled, 0, 0, 6, szTemp, FONT_NORMAL, 0, 1); 

  delay(2000);
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (myIMU.enableRotationVector() == true) {
    Serial.println(F("Rotation vector enabled"));
    Serial.println(F("Output in form i, j, k, real, accuracy"));
  } else {
    Serial.println("Could not enable rotation vector");
  }
}

void loop() {
  delay(10);

  if (lastPowerState)  // Are we "On"?
  {

    if (myIMU.wasReset()) {
      Serial.print("sensor was reset ");
      setReports();
    }

    // Has a new event come in on the Sensor Hub Bus?
    if (myIMU.getSensorEvent() == true) {

      // is it the correct sensor data we want?
      if (myIMU.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR) {

        float quatI = myIMU.getQuatI();
        float quatJ = myIMU.getQuatJ();
        float quatK = myIMU.getQuatK();
        float quatReal = myIMU.getQuatReal();
        float quatRadianAccuracy = myIMU.getQuatRadianAccuracy();
        // Calculate the orientation in the 3D Space
        Serial.print(quatI, 2);
        Serial.print(F(","));
        Serial.print(quatJ, 2);
        Serial.print(F(","));
        Serial.print(quatK, 2);
        Serial.print(F(","));
        Serial.print(quatReal, 2);
        Serial.print(F(","));
        Serial.print(quatRadianAccuracy, 2);
        sendDataToSensy(quatI, quatJ, quatK, quatReal, quatRadianAccuracy);
        sendDataToLcd(quatI, quatJ, quatK, quatReal, quatRadianAccuracy);
        Serial.println();
      }
    }
  }

  //Check if it is time to change the power state
  if (millis() > (lastMillis + 5000))  // Change state every 5 seconds
  {
    lastMillis = millis();  // Keep track of time

    if (lastPowerState)  // Are we "On"?
    {
      Serial.println("Putting BNO08x to sleep...");
      myIMU.modeSleep();  // Put BNO to sleep
    } else {
      Serial.println("Waking up BNO08x");
      myIMU.modeOn();  // Turn BNO back on
    }

    lastPowerState ^= 1;  // Invert lastPowerState (using ex-or)
  }
}
