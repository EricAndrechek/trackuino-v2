from redis import Redis
from rq import Queue
import paho.mqtt.client as mqtt
from paho.mqtt.packettypes import PacketTypes
from paho.mqtt.properties import Properties
import json

def add_data_task(data_type, data):
    client = mqtt.Client()
    client.connect("localhost", 1883, 60)
    properties=Properties(PacketTypes.PUBLISH)
    properties.MessageExpiryInterval=60 # retain messages for x seconds

    if data_type == "position":
        client.publish("POSITION/" + data["name"], json.dumps(data), retain=True, qos=0, properties=properties)
    elif data_type == "telemetry":
        client.publish("TELEMETRY/" + data["name"], json.dumps(data), retain=True, qos=0, properties=properties)
    
    client.disconnect()
    

def add_data(data_type, data):
    q = Queue(connection=Redis())
    q.enqueue(add_data, data_type, data)
