#define SerialMon Serial
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

#define version "1.1.1"

// ---------- SETTINGS ----------

// umich-balloons mqtt broker address
String broker = "mqtt.umich-balloons.com";

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]  = "hologram";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// ---------- END SETTINGS ----------

#include <TinyGsmClient.h>
#include <PubSubClient.h>

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient  mqtt(client);

#define UART_BAUD           9600
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4

#define LED_PIN             12

int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;

int ledStatus = LOW;

// battery data
int8_t   percent     = -99;
uint16_t milliVolts  = -9999;
uint8_t  chargeState = 0;

// gps data
float lat       = 0;
float lon       = 0;
float cse       = 0;
float alt       = 0;
float speed     = 0;
int   vsat      = 0;

// ---------- DISPLAY FUNCTIONS ----------

void show(String message) {
    SerialMon.println(message);
}

// ---------- END DISPLAY FUNCTIONS ----------


// ---------- MQTT FUNCTIONS ----------

// mqtt global variables

uint32_t lastReconnectAttempt = 0;

// set automatically by modem
String IMEI = "";
String ICCID = "";
String IMSI = "";

// topic for publishing
String push_string = "";

// last will and testement message, 0 for offline, 1 for online
String lwt_msg = "0"; // should auto set to 0 when offline

// callback function for mqtt
void mqttCallback(char *topic, byte *payload, unsigned int len) {
    SerialMon.print("Message arrived [");
    SerialMon.print(topic);
    SerialMon.print("]: ");
    SerialMon.write(payload, len);
    SerialMon.println();
}

// publish content String to topic
void publishTopic(String topic, String content, bool retain = false) {
  String topic2 = push_string + topic;
  mqtt.publish(topic2.c_str(), content.c_str(), retain);
}

// publisher cache
// (avoid publishing the same data multiple times if it hasn't changed)
String last_csq = "";
String last_lat = "";
String last_lon = "";
String last_bat = "";
String last_bmv = "";
String last_cse = "";
String last_spd = "";
String last_alt = "";
String last_vsat = "";

bool boot_topics = false;

// publish topics loop handler
// publishes topics with new data to broker
void publishTopics() {
    if (!mqtt.connected()) {
        return;
    }

    // always update status
    publishTopic("lwt", "1");

    if (!boot_topics) {
      // send IMEI, ICCID, and IMSI to broker
      publishTopic("IMEI", IMEI, true);
      publishTopic("ICCID", ICCID, true);
      publishTopic("IMSI", IMSI, true);
      // ^ can do this here because we know it won't change
      boot_topics = true;
    }

    // check if data has changed
    String S_csq = String(modem.getSignalQuality());
    String S_lat = String(lat, 6);
    String S_lon = String(lon, 6);
    String S_bat = String(percent);
    String S_bmv = String(milliVolts);
    String S_spd = String(speed, 2);
    String S_alt = String(alt, 2);
    String S_cse = "";
    String S_vsat = String(vsat);

    // calculate course with change in lat/lon
    if (last_lat == "" || last_lon == "") {
        last_lat = S_lat;
        last_lon = S_lon;
    }
    float last_lat_f = last_lat.toFloat();
    float last_lon_f = last_lon.toFloat();

    float dlat = S_lat.toFloat() - last_lat_f;
    float dlon = S_lon.toFloat() - last_lon_f;

    if (dlat == 0 && dlon == 0) {
        // no change in position
        S_cse = last_cse;
    } else {
        // calculate course
        cse = atan2(dlat, dlon) * 180 / PI;
        if (cse < 0) {
            cse += 360;
        }
    }
    S_cse = String(cse, 2);

    if (S_csq != last_csq) {
        publishTopic("csq", S_csq, true);
        last_csq = S_csq;
    }
    if (S_lat != last_lat) {
        publishTopic("lat", S_lat, true);
        last_lat = S_lat;
    }
    if (S_lon != last_lon) {
        publishTopic("lon", S_lon, true);
        last_lon = S_lon;
    }
    if (S_bat != last_bat) {
        publishTopic("b%", S_bat, true);
        last_bat = S_bat;
    }
    if (S_bmv != last_bmv) {
        publishTopic("bmV", S_bmv, true);
        last_bmv = S_bmv;
    }
    if (S_cse != last_cse) {
        publishTopic("cse", S_cse, true);
        last_cse = S_cse;
    }
    if (S_spd != last_spd) {
        publishTopic("spd", S_spd, true);
        last_spd = S_spd;
    }
    if (S_alt != last_alt) {
        publishTopic("alt", S_alt, true);
        last_alt = S_alt;
    }
    if (S_vsat != last_vsat) {
        publishTopic("vsat", S_vsat, true);
        last_vsat = S_vsat;
    }
}

boolean mqttConnect() {
    SerialMon.print("Connecting to ");
    SerialMon.print(broker);

    // Connect to MQTT Broker
    String lwt_path = push_string + "lwt";
    boolean status = mqtt.connect(push_string.c_str(), lwt_path.c_str(), 0, true, lwt_msg.c_str());

    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println(" success");

    publishTopics();

    return mqtt.connected();
}

// ---------- END MQTT FUNCTIONS ----------



// ---------- GPS FUNCTIONS ----------

void enableGPS(void) {
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    // send AT+CGNSNMEA=237279
    // wtf does this do??
    // modem.sendAT("+CGNSNMEA=237279");
    // and this? - 10m accuracy I think?
    // modem.sendAT("+CGNSHOR=10");
    // whats the difference between these:??
    modem.sendAT("+CGPIO=0,48,1,1");
    // modem.sendAT("+SGPIO=0,4,1,1");
    if (modem.waitResponse(10000L) != 1) {
        show("Set GPS Power HIGH Failed");
    }
    modem.enableGPS();
}

void disableGPS(void) {
    // Set Modem GPS Power Control Pin to LOW ,turn off GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        show("Set GPS Power LOW Failed");
    }
    modem.disableGPS();
}

// get latest GPS and GSM position data
void getPos() {
    bool gpsSuccess = false;
    for (int i = 0; i < 10; i++) {
        if (modem.getGPS(&lat, &lon, &speed, &alt, &vsat)) {
            break;
        }
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(2000);
    }
    if (!gpsSuccess) {
        modem.getGsmLocation(&lat, &lon);
        show("Failed to get GPS data\n");
    }
}

// ---------- END GPS FUNCTIONS ----------


// ---------- MODEM FUNCTIONS ----------

void modemPowerOn() {
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1000);    //Datasheet Ton mintues = 1S
    digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff() {
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1500);    //Datasheet Ton mintues = 1.2S
    digitalWrite(PWR_PIN, HIGH);
}

// ---------- END MODEM FUNCTIONS ----------

// function that gets called in loop to do all the gps, battery, etc
void doProcessing() {
    // handle gsm/gps
    getPos();
    
    // handle battery
    modem.getBattStats(chargeState, percent, milliVolts);

    // publish data to broker
    publishTopics();
}

void setup() {
    // Set console baud rate
    SerialMon.begin(115200);

    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // power up modem
    modemPowerOn();

    // begin serial interface to modem
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // display startup message
    String bootmsg = "Vehicle Tracker v" + String(version) + " starting up...\n";
    show(bootmsg);
    show("Firmware flash date: " + String(__DATE__) + " " + String(__TIME__) + "\n");

    show("\nChecking if Modem is online");
    //test modem is online ?
    uint32_t  timeout = millis();
    while (!modem.testAT()) {
        show(".");
        if (millis() - timeout > 60000 ) {
            show("\nModem not responding, trying to restart...\n");
            modemPowerOff();
            delay(5000);
            modemPowerOn();
            timeout = millis();
        }
    }
    show("\nModem is online\n");

    //test sim card is online ?
    timeout = millis();
    show("\nGetting SIM card status\n");
    while (modem.getSimStatus() != SIM_READY) {
        show(".");
        if (millis() - timeout > 60000 ) {
            show("\nSIM card not detected. Has it been inserted?\n");
            show("If you have inserted the SIM card, please turn off and back on and try again!\n");
            return;
        }

    }
    show("\nSIM card exists\n");

    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && modem.getSimStatus() != 3 ) {
        modem.simUnlock(GSM_PIN);
    }

    modem.sendAT("+CFUN=0 ");
    if (modem.waitResponse(10000L) != 1) {}
    delay(200);

    IMEI = modem.getIMEI();
    show("IMEI: " + IMEI);

    //Set mobile operation band
    modem.sendAT("+CBAND=ALL_MODE");
    modem.waitResponse();

    // Args:
    // 1 CAT-M
    // 2 NB-IoT
    // 3 CAT-M and NB-IoT
    modem.setPreferredMode(3);

    // Args:
    // 2 Automatic
    // 13 GSM only
    // 38 LTE only
    // 51 GSM and LTE only
    modem.setNetworkMode(38);

    delay(200);

    modem.sendAT("+CFUN=1 ");
    if (modem.waitResponse(10000L) != 1) {}
    delay(200);

    SerialAT.println("AT+CGDCONT?");
    delay(500);
    if (SerialAT.available()) {
        input = SerialAT.readString();
        for (int i = 0; i < input.length(); i++) {
            if (input.substring(i, i + 1) == "\n") {
                pieces[counter] = input.substring(lastIndex, i);
                lastIndex = i + 1;
                counter++;
            }
            if (i == input.length() - 1) {
                pieces[counter] = input.substring(lastIndex, i);
            }
        }
        // Reset for reuse
        input = "";
        counter = 0;
        lastIndex = 0;

        for ( int y = 0; y < numberOfPieces; y++) {
            for ( int x = 0; x < pieces[y].length(); x++) {
                char c = pieces[y][x];  //gets one byte from buffer
                if (c == ',') {
                    if (input.indexOf(": ") >= 0) {
                        String data = input.substring((input.indexOf(": ") + 1));
                        if ( data.toInt() > 0 && data.toInt() < 25) {
                            modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
                        }
                        input = "";
                        break;
                    }
                    // Reset for reuse
                    input = "";
                } else {
                    input += c;
                }
            }
        }
    } else {
        show("Failed to get PDP!\n");
    }

    show("\n\n\nWaiting for network...\n");
    if (!modem.waitForNetwork()) {
        delay(10000);
        return;
    }

    if (modem.isNetworkConnected()) {
        show("Network connected\n");
    }

    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        delay(10000);
        return;
    }

    if (modem.isGprsConnected()) {
        show("GPRS connected\n");
    } else {
        show("GPRS not connected\n");
    }

    show("Connecting to MQTT broker: " + broker + "\n");
    mqtt.setServer(broker.c_str(), 1883);
    mqtt.setCallback(mqttCallback);

    show("Enabling GPS\n");
    enableGPS();

    delay(1000);

    show("Setup complete\n");
    delay(500);

    ICCID = modem.getSimCCID();
    // cut last digit off of ICCID (always shows "f") ?
    ICCID = ICCID.substring(0, ICCID.length() - 1);
    IMSI = modem.getIMSI();


    // set mqtt topic root strings
    // set "SIM" to last 5 digits of ICCID
    String SIM = ICCID.substring(ICCID.length() - 5);
    push_string = "T/" + SIM + "/";

    show("Done SIM setup, starting...");

    delay(500);
}

void loop() {
    // handle network and mqtt connections
    if (!modem.isNetworkConnected()) {
        show("Network disconnected\n");

        if (!modem.waitForNetwork(180000L, true)) {
            show(" fail\n");
            delay(10000);
            return;
        }

        if (modem.isNetworkConnected()) {
            show("Network re-connected\n");
        }

        if (!modem.isGprsConnected()) {
            show("GPRS disconnected!\n");
            show("Connecting to " + String(apn) + "\n");

            if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
                show(" fail\n");
                delay(10000);
                return;
            }

            if (modem.isGprsConnected()) {
                show("GPRS reconnected\n");
            }
        }
    }

    doProcessing();

    if (!mqtt.connected()) {
        show("=== MQTT NOT CONNECTED ===\n");
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 10000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
            }
        }
        delay(100);
        return;
    }

    mqtt.loop();
}
