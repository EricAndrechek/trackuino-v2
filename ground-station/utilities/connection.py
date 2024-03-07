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

        if config.telemetry.enabled:
            # run daemon thread to send telemetry every config.telemetry.telemetry_interval seconds
            
            from threading import Thread
            self.telemetry_thread = Thread(target=self.telemetry_daemon, daemon=True, name="Telemetry Daemon")
            self.telemetry_thread.daemon = True
            self.telemetry_thread.start()

    def on_message(self, client, userdata, msg):
        print(parse_topic(msg.topic) + " " + msg.payload.decode())
    
    def on_connect(self, client, userdata, flags, rc):
        print("Connected - " + parse_rc(rc))

    def on_disconnect(self, client, userdata, rc):
        print("Disconnected - " + parse_rc(rc))
    
    def send_telemetry(self):
        cpu = "{:.2f}".format(psutil.cpu_percent()) + "%"
        memory = "{:.2f}".format(psutil.virtual_memory().percent) + "%"
        total_memory = humanfriendly.format_size(psutil.virtual_memory().total)
        disk = "{:.2f}".format(psutil.disk_usage('/').percent) + "%"
        total_disk = humanfriendly.format_size(psutil.disk_usage('/').total)
        network = humanfriendly.format_size(psutil.net_io_counters().bytes_sent + psutil.net_io_counters().bytes_recv)
        uptime = humanfriendly.format_timespan(time.mktime(time.localtime()) - psutil.boot_time())

        self.client.publish(f"{self.station}/cpu", cpu, qos=0, retain=True)
        self.client.publish(f"{self.station}/memory", memory, qos=0, retain=True)
        self.client.publish(f"{self.station}/total-memory", total_memory, qos=0, retain=True)
        self.client.publish(f"{self.station}/disk", disk, qos=0, retain=True)
        self.client.publish(f"{self.station}/total-disk", total_disk, qos=0, retain=True)
        self.client.publish(f"{self.station}/network", network, qos=0, retain=True)
        self.client.publish(f"{self.station}/uptime", uptime, qos=0, retain=True)

    def telemetry_daemon(self):
        while True:
            self.send_telemetry()
            time.sleep(config.telemetry.telemetry_interval)


# Main Loop

if __name__ == "__main__":
    client = GS_Client()
    while True:
        time.sleep(1)