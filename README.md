# Sensy32 Board Library

## Description
The Sensy32 Sensor Board Library provides a comprehensive set of code snippets to interface with the various sensors and components present on the Sensy32 board. This library simplifies the process of integrating the sensors into your projects.

## Components / Sensors
The Sensy32 board includes the following components and sensors:
- UV Light Sensor
- IR Motion and Human Presence Sensor
- Humidity and Temperature Sensor
- Altitude and Pressure Sensor
- 9-DOF Orientation IMU Sensor (Accelerometer/Magnetometer/Gyro)
- Microcontroller (ESP32-S3 Wi-Fi and Bluetooth)
- 32Mb Nor Flash Memory
- USB to Serial Converter
- Light Intensity Sensor
- Battery Charger
- RGB LEDs
- MEMS Microphone

## Documentation
You can use the [Sensy32 Documentation](https://sensy32.io/documentation) to get all information about this project, Or also follow this guide step by step:

### Prerequisites
1. Make sure to install both the **Arduino IDE** software and the **CP210xVCP Driver**:
    - Arduino IDE: [Download from here](https://www.arduino.cc/en/software)
    - CP210xVCP Driver: [Download from here](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
2. Ensure you have a Sensy32 board. If you don’t have one yet, you can get it from here: [Get from here](https://www.tindie.com/products/sensy32/sensy32-all-in-one-sensor-iot-board-with-lcd/?pt=ac_prod_search)
   
### Arduino IDE Setup
3. Click on the **Arduino IDE** tab in the top-left corner, then select **Settings**.
   
<img src="https://sensy32.io/static/media/arduino-setup-1.c08d3b8742880add3e40.png" alt="arduino setup 1" width="200">

In the **Settings** menu, add the following link to the **Additional Boards Manager URLs** field: [https://dl.espressif.com/dl/package_esp32_index.json](https://dl.espressif.com/dl/package_esp32_index.json)

After adding the link, click the **OK** button.

<img src="https://sensy32.io/static/media/arduino-setup-2.8fa98ce4fe8e2d822a0c.png" alt="arduino setup 2" width="600">

4. Next, download the ESP32 board manager by navigating to the **Boards Manager** under the second menu tab (vertically). Look for the **ESP32 by Espressif Systems** (it's the second one in the list) and install it.

<img src="https://sensy32.io/static/media/arduino-setup-3.b3ba0457f669c63d694e.png" alt="arduino setup 3" width="300">

### Code Execution
5. Open a new sketch in the Arduino IDE and copy-paste the content from one of the following files, depending on the measurement you want to perform:
    - Temperature, Humidity, Altitude, and Pressure: use the file **temperature-humidity-sensor.ino**.
    - Altitude, Pressure, and Temperature: use the file **pressure-altitude-sensor.ino**.
    - UV light value: use the file **uv-sensor.ino**.
    - Orientation (quatI, quatJ, quatK, quatReal, quatRadianAccuracy): use the file **orientation-sensor.ino**.
    - IR motion and human presence: use the file **motion-sensor.ino**.
    - Light Intensity(IR, Full, Visible, Lux): use the file **light-sensor.ino**.
    - Accelerometer(X, Y, Z): use the file **accelerometer-sensor.ino**.
    - All Measurements at Once: use the file **all_sensors.ino**.

6. Now, connect your Sensy32 board to your computer using a USB Type-C cable.

***Note***: This command may help you detect whether the board is successfully connected. It shows a list of specific devices or files related to serial communication on your system: **ls /dev/cu.\***

7. Next, click on the Sketch tab in the Arduino IDE, select Add File, and upload the **config.h** file as demonstrated in the screenshots below:
   
<p>
   <img src="https://sensy32.io/static/media/code-execution-1.0dbca2b8964290f26da1.png" alt="code execution 1" width="300">
    <img src="https://sensy32.io/static/media/code-execution-2.9f30fb88c97f9b37ab5e.png" alt="code execution 2" width="600"> 
</p>

8. Now, update the following variables in the config.h file:
    - **apiKey**: Replace it with your board's API key (obtained when you create a new board on [https://sensy32.io](https://sensy32.io)).
    - **ssid** and **password**: Replace them with your Wi-Fi credentials.

<img src="https://sensy32.io/static/media/code-execution-3.acba995c8cc802ac5e12.png" alt="code execution 3" width="600"> 

9. Now, download the required packages specified in the config.h file and any additional dependencies referenced in the other file you're using.

<p>
   <img src="https://sensy32.io/static/media/code-execution-4.3c46f6d2a714be3588b6.png" alt="code execution 4" width="400">
   <img src="https://sensy32.io/static/media/code-execution-5.d91cb139635e5dcc4461.png" alt="code execution 5" width="600"> 
</p>

Some packages are pre-installed, and you only need to install the missing ones through the **Library Manager** (accessible from the third menu tab).

***Notes***: 
- These packages only need to be installed once. For instance, if you initially run the **all_sensors.ino** file, you won’t need to install additional packages when switching to **temperature-humidity-sensor.ino** or any other individual sensor file.
- In the rare case you’re prompted for other packages, simply search for their names in the Library Manager and install them as shown in the examples. However, this is usually unnecessary.

<p>
   <img src="https://sensy32.io/static/media/package-1.befda9f6c9d7e44b298b.png" alt="package 1" width="200">
   <img src="https://sensy32.io/static/media/package-2.cfe17c28bef19db4cf2f.png" alt="package 2" width="200"> 
   <img src="https://sensy32.io/static/media/package-3.d9e02ffe6188fd0310ea.png" alt="package 3" width="200">
   <img src="https://sensy32.io/static/media/package-4.d94a872d03730060b09f.png" alt="package 4" width="200"> 
</p>

<p>
   <img src="https://sensy32.io/static/media/package-5.62db9a614d727a2db95f.png" alt="package 5" width="200">
   <img src="https://sensy32.io/static/media/package-6.09e008de9508f94b9cff.png" alt="package 6" width="200"> 
   <img src="https://sensy32.io/static/media/package-7.7e944b15cb860261988f.png" alt="package 7" width="200">
   <img src="https://sensy32.io/static/media/package-8.b6043606098430e3f853.png" alt="package 8" width="200"> 
</p>

If prompted to install library dependencies (as shown in the example below), simply click **Install All** to ensure all necessary libraries are installed.

<img src="https://sensy32.io/static/media/install-all.175d11782d8f86791f49.png" alt="install all" width="500">

Here is an overview about the role of each package used in our code samples:

- **WiFi.h** and **WiFiClientSecure.h**: These two libraries enable secure connection to and management of Wi-Fi networks.
- **Wire.h**: Used for I2C communication with the sensor.
- **HTTPClient.h**: This library provides support for making HTTP requests and responses.
- **ss_oled.h**: This library is used to configure and display data on the LCD screen.
- **ArduinoJson.h**: This library is used for serializing and deserializing JSON objects to facilitate data exchange with the server.
- **SparkFunBME280.h**: This library helps work with the SparkFun BME280 sensor, allowing you to read temperature, humidity, and pressure data.
- **BMP388_DEV.h**: This library facilitates interaction with the BMP388 sensor module.
- **Adafruit_LTR390.h**: A library for communicating with the Adafruit_LTR390 UV sensor.
- **Adafruit_TSL2591.h**: A library for communicating with the Adafruit_TSL2591 Light sensor.
- **Adafruit_Sensor.h**: Provides common functionality for working without various sensors, simplifying sensor integration in projects.
- **SparkFun_BNO08x_Arduino_Library.h**: Simplifies interfacing with BNO08x series sensors in Arduino projects.
- **SparkFun_STHS34PF80_Arduino_Library.h**: Simplifies interfacing with the STHS34PF80 sensor in Arduino projects.

10. In the top-left corner, click the **Select Board** button, then choose **Select Other Board and Port** from the dropdown menu. This will open a Popup where you need to:
    - Select **ESP32S3 Dev Module** as the board.
    - Select **/dev/cu.SLAB_USBtoUART Serial Port (USB)** as the port.
    - Click the **OK** button.

<p>
   <img src="https://sensy32.io/static/media/code-execution-6.e14bf524b7e4d9cc4da0.png" alt="code execution 6" width="400">
   <img src="https://sensy32.io/static/media/code-execution-7.6c90fd610170f3c613bb.png" alt="code execution 7" width="600"> 
</p>

11. Additionally, open the **Serial Monitor** console from the **Tools** tab to view the logs in real time.

<img src="https://sensy32.io/static/media/code-execution-8.f1ec9263d25373fd9932.png" alt="code execution 8" width="500">

12. Finally, click the **Upload** button (the arrow icon) on the navigation bar to upload the code to your board.

<img src="https://sensy32.io/static/media/code-execution-9.673bee4559e96c707091.png" alt="code execution 9" width="500">

Once the upload is complete:

- **Serial Monitor**: Displays real-time values.
- **Board’s LCD Screen**: Shows the sensor data.
- **Real-Time Data Page** on [Sensy32.io](https://sensy32.io): Displays convenient charts with the data.

Get ready to watch it all in action! 🎉
    
## Contributing
Contributions to this project are welcome! If you have ideas for improvements or new features, feel free to open an issue or submit a pull request on [GitHub](https://github.com/sensy32).

## License
This project is licensed under the MIT License.

## Contact
For any inquiries or support, please contact us at:
- Email: support@sensy32.io
- Website: [Sensy32](https://sensy32.io/)
