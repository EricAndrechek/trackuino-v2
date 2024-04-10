#define SerialMon Serial
#define SerialAT Serial1

#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

#define version "1.0.0"

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
#include <TinyGPS++.h>
#include <PubSubClient.h>
#include <Wire.h>

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
uint8_t  chargeState = -99;
int8_t   percent     = -99;
uint16_t milliVolts  = -9999;

// gps data
float lat       = 0;
float lon       = 0;
float spd       = 0;
float alt       = 0;
float cse       = 0;
int   vsat      = 0;
float usat      = 0;
float acc       = 0;
int   year      = 0;
int   month     = 0;
int   day       = 0;
int   hour      = 0;
int   minute    = 0;
int   second    = 0;
String hours    = "";
String minutes  = "";
String seconds  = "";
String months   = "";
String days     = "";
String years    = "";

// ---------- DISPLAY FUNCTIONS ----------

// if debugging is enabled, print to serial monitor as well
#define DEBUG true

// if display is enabled, print to display as well
#define Display true

void show(String message) {
    #if Display
        Wire.beginTransmission(0x3C);
        Wire.write(message.c_str());
        Wire.endTransmission();
    #endif
    #if DEBUG
        SerialMon.println(message);
    #endif
}

void clearDisplay() {
    #if Display
        Wire.beginTransmission(0x3C);
        Wire.write("*CLEAR*");
        Wire.endTransmission();
    #endif
}

void initCompass() {
    #if Display
        Wire.beginTransmission(0x3C);
        Wire.write("*INIT*");
        Wire.endTransmission();
    #endif
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

// topic for subscribing
String sub_string = "";

// subscription routes
String sub_dir = "B/";
char* sub_routes[] = {"cLa", "cLo", "alt", "eLa", "eLo"};

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
String last_spd = "";
String last_alt = "";
String last_cse = "";
String last_bat = "";
String last_bmv = "";
String last_bcs = "";
String last_hours = "";
String last_minutes = "";
String last_seconds = "";
String last_months = "";
String last_days = "";
String last_years = "";

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
    String S_lat = String(lat);
    String S_lon = String(lon);
    String S_spd = String(spd);
    String S_alt = String(alt);
    String S_cse = String(cse);
    String S_bat = String(percent);
    String S_bmv = String(milliVolts);
    String S_bcs = String(chargeState);

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
    if (S_spd != last_spd) {
        publishTopic("spd", S_spd, true);
        last_spd = S_spd;
    }
    if (S_alt != last_alt) {
        publishTopic("alt", S_alt, true);
        last_alt = alt;
    }
    if (S_cse != last_cse) {
        publishTopic("cse", S_cse, true);
        last_cse = cse;
    }
    if (S_bat != last_bat) {
        publishTopic("b%", S_bat, true);
        last_bat = S_bat;
    }
    if (S_bmv != last_bmv) {
        publishTopic("bmV", S_bmv, true);
        last_bmv = S_bmv;
    }
    if (S_bcs != last_bcs) {
        publishTopic("bCS", S_bcs, true);
        last_bcs = S_bcs;
    }
    if (last_months != months) {
        publishTopic("MM", months, true);
        last_months = months;
    }
    if (last_days != days) {
        publishTopic("DD", days, true);
        last_days = days;
    }
    if (last_years != years) {
        publishTopic("YY", years, true);
        last_years = years;
    }
    if (last_hours != hours) {
        publishTopic("hh", hours, true);
        last_hours = hours;
    }
    if (last_minutes != minutes) {
        publishTopic("mm", minutes, true);
        last_minutes = minutes;
    }
    if (last_seconds != seconds) {
        publishTopic("ss", seconds, true);
        last_seconds = seconds;
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

    // Subscribe to topics
    for (int i = 0; i < sizeof(sub_routes) / sizeof(sub_routes[0]); i++) {
          String topic = sub_string + sub_routes[i];
          mqtt.subscribe(topic.c_str());
    }
    publishTopics();

    return mqtt.connected();
}

// ---------- END MQTT FUNCTIONS ----------



// ---------- GPS FUNCTIONS ----------

TinyGPSPlus gps;

void enableGPS(void) {
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,1");
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

    // GSM
    modem.getGsmLocation(&lat, &lon, &acc, &year, &month, &day, &hour, &minute, &second);


    // GPS
    String raw_gps = modem.getGPSraw();
    // Parse GPS data with TinyGPS++
    for (int i = 0; i < raw_gps.length(); i++) {
        gps.encode(raw_gps[i]);
    }
    // check that gps data is valid and age is less than 5 seconds old before using it
    if (gps.location.isValid() && gps.location.age() < 5000) {
        lat = gps.location.lat();
        lon = gps.location.lng();
        spd = gps.speed.kmph();
        alt = gps.altitude.meters();
        cse = gps.course.deg();
        vsat = gps.satellites.value();
        usat = gps.hdop.hdop();
        acc = gps.hdop.hdop();
        year = gps.date.year();
        month = gps.date.month();
        day = gps.date.day();
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
    }

    // put date and times into their strings with leading zeros
    if (hour < 10) {
        hours = "0" + String(hour);
    } else {
        hours = String(hour);
    }
    if (minute < 10) {
        minutes = "0" + String(minute);
    } else {
        minutes = String(minute);
    }
    if (second < 10) {
        seconds = "0" + String(second);
    } else {
        seconds = String(second);
    }
    if (month < 10) {
        months = "0" + String(month);
    } else {
        months = String(month);
    }
    if (day < 10) {
        days = "0" + String(day);
    } else {
        days = String(day);
    }
    years = String(year);
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

    #if Display
        // communicate with display over I2C
        Wire.begin();
        clearDisplay();
    #endif

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
    sub_string = "B/" + SIM + "/";

    show("Done SIM setup, starting...");

    delay(500);

    initCompass();
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
    show("PLZ?");

    mqtt.loop();
}
