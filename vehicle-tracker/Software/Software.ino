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

// battery data
uint8_t  chargeState = -99;
int8_t   percent     = -99;
uint16_t milliVolts  = -9999;

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

uint32_t lastReconnectAttempt = 0;


// ---------- MQTT FUNCTIONS ----------

// mqtt global variables

// set automatically by modem
String IMEI = "";

// topic for publishing
String push_string = "";

// subscription routes
String sub_dir = "B/";
char* sub_routes[] = {"cLa", "curLon", "alt", "endLat", "endLon"};

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
void publishTopic(String topic, String content) {
  String topic2 = "VT/" + IMEI + "/VehicleTelemetry/" + topic;
  mqtt.publish(topic2.c_str(), content.c_str());
}

// publish topics loop handler
// publishes topics with new data to broker
void publishTopics() {
    if (!mqtt.connected()) {
        return;
    }
    modem.getBattStats(chargeState, percent, milliVolts);
    String bat_perc = String(percent) + "%";
    float temp = modem.getTemperature();

    String lat = String(lat2) if lat2 != 0 else String(lat);
    String lon = String(lon2) if lon2 != 0 else String(lon);
    String time2 = String(hour2) + ":" + String(min2) + ":" + String(sec2);
    String time1 = String(hour) + ":" + String(min) + ":" + String(sec);
    String time = time2 if time2 != "0:0:0" else time1;

    publishTopic("csq", String(modem.getSignalQuality()));
    publishTopic("lwt", "Online");
    publishTopic("lat", lat);
    publishTopic("lon", String(lon2, 8));
    publishTopic("spd", String(speed2));
    publishTopic("alt", String(alt2));
    publishTopic("bat", bat_perc);
    publishTopic("tmp", String(temp))
}

boolean mqttConnect() {
    SerialMon.print("Connecting to ");
    SerialMon.print(broker);

    // Connect to MQTT Broker
    String root_dir = "VehicleTrackers/" + IMEI;
    String lwt_path = root_dir + "/VehicleTelemetry/lwt";
    boolean status = mqtt.connect(root_dir.c_str(), lwt_path.c_str(), 0, true, lwt_msg.c_str());

    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println(" success");

    // Subscribe to topics
    for (int i = 0; i < sizeof(sub_routes) / sizeof(sub_routes[0]); i++) {
          String topic = "VehicleTrackers/" + IMEI + "/Balloon/" + sub_routes[i];
          mqtt.subscribe(topic.c_str());
    }
    publishTopics();

    return mqtt.connected();
}

// ---------- END MQTT FUNCTIONS ----------



// ---------- GPS FUNCTIONS ----------

TinyGPSPlus gps;

// gsm data
float gsm_lat       = 0;
float gsm_lon       = 0;
float gsm_acc       = 0;
int   gsm_year      = 0;
int   gsm_month     = 0;
int   gsm_day       = 0;
int   gsm_hour      = 0;
int   gsm_min       = 0;
int   gsm_sec       = 0;

// gps data
float gps_lat       = 0;
float gps_lon       = 0;
float gps_speed     = 0;
float gps_alt       = 0;

int   gps_vsat      = 0;
float gps_usat      = 0;
float gps_accuracy  = 0;
int   gps_year      = 0;
int   gps_month     = 0;
int   gps_day       = 0;
int   gps_hour      = 0;
int   gps_min       = 0;
int   gps_sec       = 0;


void enableGPS(void) {
    // Set Modem GPS Power Control Pin to HIGH ,turn on GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power HIGH Failed");
    }
    modem.enableGPS();
}

void disableGPS(void) {
    // Set Modem GPS Power Control Pin to LOW ,turn off GPS power
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+CGPIO=0,48,1,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("Set GPS Power LOW Failed");
    }
    modem.disableGPS();
}

// get latest GPS and GSM position data
void getPos() {
    // GPS
    String raw_gps = modem.getGPSraw();
    // Parse GPS data with TinyGPS++
    gps.encode(raw_gps);
    if (gps.location.isValid()) {
        lat2 = gps.location.lat();
        lon2 = gps.location.lng();
        speed2 = gps.speed.kmph();
        alt2 = gps.altitude.meters();
        vsat2 = gps.satellites.value();
        usat2 = gps.hdop.hdop();
        accuracy2 = gps.hdop.hdop();
        year2 = gps.date.year();
        month2 = gps.date.month();
        day2 = gps.date.day();
        hour2 = gps.time.hour();
        min2 = gps.time.minute();
        sec2 = gps.time.second();
    }

    // GSM
    modem.getGsmLocation(&gsm_lat, &gsm_lon, &gsm_acc, &gsm_year, &gsm_month, &gsm_day, &gsm_hour, &gsm_min, &gsm_sec);
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


// ---------- DISPLAY FUNCTIONS ----------

// if debugging is enabled, print to serial monitor as well
#define DEBUG true

// if display is enabled, print to display as well
#define Display false

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
        Wire.setClock(400000);
        clearDisplay();
    #endif

    // display startup message
    show("Vehicle Tracker v" + version + " starting up...\n");
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
    int IMEI_num = IMEI.toInt();
    show("IMEI: " + IMEI);
    push_string = "VT/" + String(IMEI_num, HEX) + "/D/";

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

    getPos();

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
