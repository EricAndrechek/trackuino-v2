import paho.mqtt.client as mqtt
import time
import psutil
import humanfriendly
try:
    from utilities.config import config
except:
    from config import config

# Helper Functions

def parse_topic(topic):
    try:
        route = topic.split("/")
        return route[0].upper() + ": " + route[1].upper() + "-" + route[2]
    except:
        return "Unknown: " + str(topic)

def parse_rc(rc):
    if rc == 0:
        return "Connection Successful"
    elif rc == 1:
        return "Connection Refused: incorrect protocol version"
    elif rc == 2:
        return "Connection Refused: invalid client identifier"
    elif rc == 3:
        return "Connection Refused: server unavailable"
    elif rc == 4:
        return "Connection Refused: bad username or password"
    elif rc == 5:
        return "Connection Refused: not authorized"
    else:
        return "Unknown: " + str(rc)
    
# MQTT Client

class GS_Client():
    def __init__(self):
        self.station = config.callsign + "-" + str(config.ssid)

        self.client = mqtt.Client(clean_session=False, client_id=self.station)

        # TODO: use TLS and auth

        self.client.connect_async(config.server.hostname, config.server.port, config.server.keepalive)
        self.client.loop_start()

        self.client.will_set(f"{self.station}/status", "offline", qos=1, retain=True)
        self.client.publish(f"{self.station}/status", "online", qos=1, retain=True)

        self.client.on_message = self.on_message
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect

    def on_message(self, client, userdata, msg):
        print(parse_topic(msg.topic) + " " + msg.payload.decode())
    
    def on_connect(self, client, userdata, flags, rc):
        print("Connected - " + parse_rc(rc))

    def on_disconnect(self, client, userdata, rc):
        print("Disconnected - " + parse_rc(rc))

# Main Loop

if __name__ == "__main__":
    client = GS_Client()
    while True:
        time.sleep(1)
