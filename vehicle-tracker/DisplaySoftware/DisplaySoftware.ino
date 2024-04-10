//This sketch is an example of reading a SHTx series temp/humidity sensor
//using i2c
//There is no library required because the communication code is in-line
//with the main loop
//this sensor is sold under the name ENV by M5Stack

#include "Arduino.h"
#include "TFT_eSPI.h"
#include <Wire.h>
#include "BootScreen.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);
#define PIN_POWER_ON 15 //power enable pin for LCD and use battery if installed
#define PIN_LCD_BL 38   // BackLight enable pin (see Dimming.txt)

bool newRxData = false;
String rx_message = "";

void receiveWire(int numBytesReceived) {
  addText("New msg!");
    while (Wire.available()) {
        rx_message += Wire.read();
    }
    newRxData = true;
}

// clear the display and set to black
void clearScreen() {
    sprite.fillSprite(TFT_BLACK);
    sprite.pushSprite(165, 0);
}

// show the boot screen
void displayBootScreen() {
    tft.fillScreen(TFT_WHITE);  //horiz / vert<> position/dimension
    tft.pushImage(0, 0, 320, 170, BootScreen);
    tft.setTextSize(1);
    sprite.createSprite(155, 170);

    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.setTextDatum(4);
}

// setup the screen to show a compass on the left, pointing in the given angle (0-360) and a text display on the right
void initCompass() {
    sprite.fillSprite(TFT_WHITE);
    sprite.pushSprite(0, 0);

    // draw compass
    sprite.fillSprite(TFT_WHITE);
    sprite.fillCircle(160, 85, 80, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160 + 80, 85, 160, 85 - 10, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160 - 80, 85, 160, 85 - 10, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160, 85 + 80, 160 + 10, 85, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160, 85 - 80, 160 + 10, 85, TFT_BLACK);

    sprite.pushSprite(0, 0);

    // draw text display
    sprite.fillSprite(TFT_WHITE);
    sprite.setTextWrap(true);
    sprite.drawString("Compass", 0, 0);
    sprite.pushSprite(0, 0);
}

// set the angle of the compass
void setCompassAngle(int angle) {
    sprite.fillSprite(TFT_WHITE);
    sprite.fillCircle(160, 85, 80, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160 + 80, 85, 160, 85 - 10, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160 - 80, 85, 160, 85 - 10, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160, 85 + 80, 160 + 10, 85, TFT_BLACK);
    sprite.fillTriangle(160, 85, 160, 85 - 80, 160 + 10, 85, TFT_BLACK);

    int x = 160 + 80 * cos(angle * 3.14159 / 180);
    int y = 85 + 80 * sin(angle * 3.14159 / 180);
    sprite.drawLine(160, 85, x, y, TFT_RED);

    sprite.pushSprite(0, 0);
}

// add a line of text to the right half of the display
// if the text is too long, it will wrap to the next line
// if the text is too long for the screen, it will scroll up
void addText(String text) {
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextWrap(true);
    sprite.drawString(text, 0, 50);
    sprite.pushSprite(170, 0);
}

void processCommand(String input) {
    if (input.startsWith("*") && input.endsWith("*") && input.length() > 2) {
        String parsed_input = input.substring(1, input.length() - 1);

        // split the input into command and arguments
        int space_index = parsed_input.indexOf(" ");
        String command = "";
        String arguments = "";
        if (space_index != -1) {
            command = parsed_input.substring(0, space_index);
            arguments = parsed_input.substring(space_index + 1);
        } else {
            command = parsed_input;
        }

        if (command == "clear") {
            clearScreen();
        } else if (command == "init") {
            initCompass();
        } else if (command == "angle") {
            setCompassAngle(arguments.toInt());
        } else if (command == "time") {
            addText("Time: " + arguments);
        } else {
            addText("Unknown command: " + command);
        }

    } else {
        input.trim();
        addText(input);
    }
}

void loop() {
    if (newRxData) {
        Serial.println(rx_message);
        processCommand(rx_message);

        newRxData = false;
        rx_message = "";
    }
}

void setup() {

    pinMode(PIN_POWER_ON, OUTPUT);  //enables the LCD and to run on battery
    pinMode(PIN_LCD_BL, OUTPUT);  //triggers the LCD backlight
    digitalWrite(PIN_POWER_ON, HIGH);
    digitalWrite(PIN_LCD_BL, HIGH);

    Serial.begin(115200);  // be sure to set USB CDC On Boot: "Enabled"

    Wire.begin(43, 44);  //SDA, SCL
    Wire.onReceive(receiveWire);

    Serial.print("In setup");

    // display setup
    tft.init();
    tft.setRotation(3);
    tft.setSwapBytes(true);

    displayBootScreen();

    delay(10000);

    pinMode(14, INPUT); //Right button proven to be pulled up, push = 0
    pinMode(0, INPUT); //Left button proven to be pulled up, push = 0

    Serial.println("Setup complete");
    clearScreen();

    addText("Setup complete");
    addText("Waiting for commands...");
}
