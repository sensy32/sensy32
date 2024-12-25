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
[//]: # "You can use the [Sensy32 Documentation](https://sensy32.io/documentation) to get all information about this project."

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

<img src="https://sensy32.io/static/media/arduino-setup-3.b3ba0457f669c63d694e.png" alt="arduino setup 2" width="200">

### Code Execution

## Contributing
Contributions to this project are welcome! If you have ideas for improvements or new features, feel free to open an issue or submit a pull request on [GitHub](https://github.com/sensy32).

## License
This project is licensed under the MIT License.

## Contact
For any inquiries or support, please contact us at:
- Email: support@sensy32.io
- Website: [Sensy32](https://sensy32.io/)
