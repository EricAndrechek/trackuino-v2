# Vehicle Tracker

The vehicle tracker is a module that can be used to track the location of a vehicle. It is designed to be used with the [balloon tracker](../balloon-tracker/readme.md) and [ground station](../ground-station/readme.md) modules. The vehicle tracker is designed to be used with a vehicle, such as a car, truck, or motorcycle. It can be used to track the location of the vehicle, and can also be used to send data to the ground station.

## Hardware

We are using the LILYGO T-SIM7000G ESP32 board for the vehicle tracker. This board includes:

- SIM7000G module, which can be used to send data over the cellular network
- GPS module, which can be used to track the location of the vehicle
- ESP32 microcontroller, which can be used to run the vehicle tracker software
- 18650 battery holder, which can be used to power the board
- USB-C port, which can be used to charge the battery and program the board
- WiFi and Bluetooth connectivity
- MicroSD card slot

Additionally, we use a LILYGO T-DISPLAY ESP32-S3R8 board for its 1.9 inch 8 bit LCD display. This display is used to show the current location of the vehicle, as well as other information such as the battery level and signal strength. The display is connected to the ESP32 microcontroller via SPI, but also includes WiFi and Bluetooth connectivity.

## Software

The vehicle tracker software is written in C++ using the Arduino framework. It is designed to run on the ESP32 microcontroller. To program the ESP32, we use the Arduino IDE with the ESP32 board support package installed.

### Programming the LILYGO T-SIM7000G ESP32

To program the LILYGO T-SIM7000G ESP32 board, follow these steps:

1. Install the Arduino IDE on your computer
2. Install the ESP32 board support package in the Arduino IDE
3. Connect the LILYGO T-SIM7000G ESP32 board to your computer using a USB-C cable
4. Select the correct board and port in the Arduino IDE
   1. The board should be "ESP32 Dev Module"
5. Open the vehicle tracker sketch in the Arduino IDE
6. Click the upload button to upload the sketch to the board
   1. If this fails, you may need to change the upload speed in the Arduino IDE. Do this by going to Tools -> Upload Speed and selecting a different speed, like 115200.
