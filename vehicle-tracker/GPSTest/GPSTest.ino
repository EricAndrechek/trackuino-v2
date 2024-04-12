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

// topic for subscribing
String sub_string = "";

// subscription routes
String sub_dir = "B/";
char* sub_routes[] = {"cLa", "cLo", "alt", "eLa", "eLo"};

// last will and testement message, 0 for offline, 1 for online
String lwt_msg = "0"; // should auto set to 0 when offline

// publish content String to topic
void publishTopic(String topic, String content, bool retain = false) {
  String topic2 = push_string + topic;
  show("Publishing to topic: " + topic2 + " with content: " + content);
}

// publisher cache
// (avoid publishing the same data multiple times if it hasn't changed)
String last_csq = "";
String last_lat = "";
String last_lon = "";
String last_spd = "";
String last_alt = "";
String last_cse = "";
String last_vsat = "";
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
    String S_lat = String(lat, 6);
    String S_lon = String(lon, 6);
    String S_spd = String(spd, 3);
    String S_alt = String(alt, 3);
    String S_cse = String(cse, 2);
    String S_vsat = String(vsat);
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
        last_alt = S_alt;
    }
    if (S_cse != last_cse) {
        publishTopic("cse", S_cse, true);
        last_cse = S_cse;
    }
    if (S_vsat != last_vsat) {
      publishTopic("vsat", S_vsat, true);
      last_vsat = S_vsat;
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

// ---------- END MQTT FUNCTIONS ----------



// ---------- GPS FUNCTIONS ----------

TinyGPSPlus gps;

void enableGPS(void) {
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    // send AT+CGNSNMEA=237279
    // wtf does this do??
    // modem.sendAT("+CGNSNMEA=237279");
    // and this? - 10m accuracy I think?
    modem.sendAT("+CGNSHOR=10");
    // whats the difference between these:??
    modem.sendAT("+CGPIO=0,48,1,1");
    modem.sendAT("+SGPIO=0,4,1,1");
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
    modem.getGsmLocation(&lon, &lat, &acc, &year, &month, &day, &hour, &minute, &second);
    SerialMon.println("GSM Location:");
    SerialMon.print("  Latitude: "); SerialMon.println(lat, 6);
    SerialMon.print("  Longitude: "); SerialMon.println(lon, 6);
    SerialMon.print("  Accuracy: "); SerialMon.println(acc);
    SerialMon.print("  Date: "); SerialMon.print(year); SerialMon.print("/"); SerialMon.print(month); SerialMon.print("/"); SerialMon.println(day);
    SerialMon.print("  Time: "); SerialMon.print(hour); SerialMon.print(":"); SerialMon.print(minute); SerialMon.print(":"); SerialMon.println(second);


    // GPS
    for (int i = 0; i < 15; i++) {
        if (modem.getGPS(&lat, &lon)) {
            Serial.println("The location has been locked, the latitude and longitude are:");
            Serial.print("latitude:"); Serial.println(lat);
            Serial.print("longitude:"); Serial.println(lon);
            break;
        }
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        delay(2000);
    }
    String raw_gps = modem.getGPSraw();
    SerialMon.println("GPS Location:");
    SerialMon.println(raw_gps);
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
    } else {
        show(String(gps.location.isValid()));
        show(String(gps.location.age()));
        show(String(gps.satellites.value()));
        vsat = gps.satellites.value();
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

    show("Enabling GPS\n");
    enableGPS();

    delay(15000);

    show("Done SIM setup, starting...");

    delay(500);
}

void loop() {
    doProcessing();
}
