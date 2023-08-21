#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial iridium(2, 3);

void send_command(String command) {
    Serial.println(">" + command);
    iridium.println(command + "\r");
    delay(1000);
    while (iridium.available()) {
        Serial.write(iridium.read());
    }
}


void setup() {
    // put your setup code here, to run once:
    iridium.begin(19200);
    Serial.begin(9600);
    Serial.println("\n\nBeginning...\n");

    send_command("AT");

    send_command("AT+SBDSX");

    send_command("AT+CGMI");

    send_command("AT+CGMM");

    send_command("AT+SBDWT=Eric is smart");

    send_command("AT+SBDIX");

    delay(10000);
    while (iridium.available()) {
        Serial.write(iridium.read());
    }

    // wait indefinitely checking for messages
    while (1) {
        send_command("AT+SBDIX");
        delay(10000);
        while (iridium.available()) {
            Serial.write(iridium.read());
        }
    }
}

void loop() {
    // put your main code here, to run repeatedly:
}
