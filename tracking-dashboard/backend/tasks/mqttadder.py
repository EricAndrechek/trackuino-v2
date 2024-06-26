from redis import Redis
from rq import Queue
import paho.mqtt.client as mqtt
import json

def add_data_task(data_type, data):
    print("adding data to mqtt")
    client = mqtt.Client()
    client.connect("localhost", 1883, 60)

    if data_type == "position":
        topic = "POSITION/" + data["name"]
        client.publish(topic, json.dumps(data), retain=True, qos=0)
    elif data_type == "telemetry":
        if "telemetry" in data:
            t_data = data["telemetry"]
        for key in t_data:
            topic = "TELEMETRY/" + data["name"] + "/" + key
            client.publish(topic, json.dumps(t_data[key]), retain=True, qos=0)

    else:
        print("Invalid data type")
    
    client.disconnect()
    

def add_data(data_type, data):
    q = Queue('api-tasks', connection=Redis())
    q.enqueue(add_data_task, data_type, data)
