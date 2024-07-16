# constant running python script to sync data from mqtt to postgres

import paho.mqtt.client as mqtt
from datetime import datetime
from sql.helpers import check_item_id, add_item_id, get_all_callsigns
from utils.api import Data
import json

# ----- MQTT SETUP -----

# open mqtt connection
client = mqtt.Client()
# client.connect("mqtt.umich-balloons.com", 1883, 60)
client.connect("localhost", 1883, 60)

# subscribe to all trackers
client.subscribe("T/#")

old_messages = {}
message_building = {}

nonTelemKeys = [
    "IMEI",
    "ICCID",
    "IMSI",
    "VER",
    "name",
    "ssid",
    "sym",
    "lat",
    "lon",
    "alt",
    "cse",
    "spd",
    "lwt",
]

# ----- FUNCTIONS -----


def build_json_message(id):
    # should have (at minimum):
    # id, name, ssid, sym, lat, lon, alt, course, speed, comment
    # optionally also have:
    # timestamp, telemetry
    message = {}
    if id not in message_building:
        print("No message_building data for id: ", id)
        return None

    if "name" in message_building[id]:
        message["callsign"] = message_building[id]["name"]
    else:
        message["callsign"] = str(id)[:6]
    if "ssid" in message_building[id]:
        message["ssid"] = message_building[id]["ssid"]
    else:
        message["ssid"] = 0
    if "sym" in message_building[id]:
        message["symbol"] = message_building[id]["sym"]
    else:
        message["symbol"] = "/>"
    if "lat" in message_building[id]:
        message["lat"] = message_building[id]["lat"]
    else:
        message["lat"] = None
    if "lon" in message_building[id]:
        message["lon"] = message_building[id]["lon"]
    else:
        message["lon"] = None
    if "alt" in message_building[id]:
        message["alt"] = message_building[id]["alt"]
    else:
        message["alt"] = None
    if "cse" in message_building[id]:
        message["course"] = message_building[id]["cse"]
    else:
        message["course"] = None
    if "spd" in message_building[id]:
        message["speed"] = message_building[id]["spd"]
    else:
        message["speed"] = None
    if "dt" in message_building[id]:
        message["timestamp"] = message_building[id]["dt"]
    else:
        message["timestamp"] = None

    # for all other keys, add them to telemetry
    telemetry = {}
    for key in message_building[id]:
        if key not in nonTelemKeys:
            telemetry[key] = message_building[id][key]

    if len(telemetry) > 0:
        message["telemetry"] = telemetry

    return message


# build a source packet of the type the API likes from a message
def build_source_packet(message):
    source = {}
    source["timestamp"] = str(datetime.now().isoformat())
    source["callsign"] = "UMSERV"
    source["ssid"] = 0
    source["ip"] = "127.0.0.1"
    source["type"] = "json"
    source["data"] = message

    return source


# call add_message on message
def on_message(client, userdata, message):
    topic = message.topic
    payload = message.payload.decode()
    timestamp = str(datetime.now().isoformat())

    # split topic
    topics = topic.split("/")
    if topics[0] == "T":
        # handle new tracker message
        # tracker messages are the only data influx to mqtt

        if len(topics) != 3:
            print("Invalid topic: ", topic)
            return

        id = topics[1]
        key = topics[2]

        # check if id is in Items table
        # if not, add it
        # we know it is in items table if "name" has been assigned
        if id in message_building and "name" in message_building[id]:
            pass
        else:
            # now check db
            item = check_item_id(id)
            if item is None:
                # check if in message_building
                if id in message_building:
                    # create item with message_building data
                    callsign = str(id)[:6]
                    ssid = 0
                    symbol = "/>"
                    if "name" in message_building[id]:
                        callsign = message_building[id]["name"]
                    if "ssid" in message_building[id]:
                        ssid = message_building[id]["ssid"]
                    if "sym" in message_building[id]:
                        symbol = message_building[id]["sym"]

                    add_item_id(id, callsign, ssid, symbol)
                else:
                    # add id to message_building and
                    message_building[id] = {}
            else:
                # add item data to message_building
                if id not in message_building:
                    message_building[id] = {}
                message_building[id]["name"] = item.callsign
                message_building[id]["ssid"] = item.ssid
                message_building[id]["sym"] = item.symbol

        # update timestamp
        message_building[id]["dt"] = timestamp

        # if key is lwt, modify telemetry data to show changed lwt
        if key == "lwt":
            if id in message_building:
                if "telemetry" in message_building[id]:
                    message_building[id]["telemetry"]["lwt"] = payload
                    # send telemetry data to mqtt
                    try:
                        topic = (
                            "TELEMETRY/"
                            + message_building[id]["name"]
                            + "-"
                            + str(message_building[id]["ssid"])
                        )
                        client.publish(
                            topic + "/lwt", json.dumps(payload), retain=True, qos=0
                        )
                    except Exception as e:
                        print("Error sending lwt to mqtt: ", e)

        # if key is "ss" (seconds), add message to db
        if key == "lat" or key == "lon" or key == "alt":
            print(key, payload)
            # add message to db
            msg = build_json_message(id)
            if msg is None:
                print("Error building message")
                return
            src = build_source_packet(msg)
            data_obj = Data()

            print("Uploading data")

            try:
                data_obj.upload(src)
                data_obj.info["ip"] = src["ip"]
            except Exception as e:
                print("Error uploading data: ", e)
                return

            print("Parsing data")

            try:
                data_obj.parse()
            except Exception as e:
                print("Error parsing data: ", e)
                return

            print("Saving data")
            print("lat" in message_building[id])
            print("lon" in message_building[id])
            print("alt" in message_building[id])

            # check if lat, lon, or alt changed
            if (
                "lat" in message_building[id]
                and "lon" in message_building[id]
                and "alt" in message_building[id]
            ):
                if id in old_messages:
                    if (
                        message_building[id]["lat"] == old_messages[id]["lat"]
                        and message_building[id]["lon"] == old_messages[id]["lon"]
                        and message_building[id]["alt"] == old_messages[id]["alt"]
                    ):
                        # send telemetry data to mqtt
                        if "telemetry" in src["data"]:
                            topic = (
                                "TELEMETRY/"
                                + message_building[id]["name"]
                                + "-"
                                + str(message_building[id]["ssid"])
                            )
                            for key in src["data"]["telemetry"]:
                                # check if last value is the same
                                if (
                                    key in old_messages[id]
                                    and src["data"]["telemetry"][key]
                                    == old_messages[id][key]
                                ):
                                    continue
                                client.publish(
                                    topic + "/" + key,
                                    json.dumps(src["data"]["telemetry"][key]),
                                    retain=True,
                                    qos=0,
                                )
                        if key == "lat":
                            message_building[id]["lat"] = payload
                        elif key == "lon":
                            message_building[id]["lon"] = payload
                        elif key == "alt":
                            message_building[id]["alt"] = payload

                try:
                    status_code = data_obj.save()
                    if status_code == 201 or status_code == 202:
                        print("New data saved")
                    elif status_code == 208:
                        print("Data already exists")
                except Exception as e:
                    print("save error: ", e)

                # copy values to old_messages
                old_messages[id] = message_building[id].copy()
                if key == "lat":
                    message_building[id]["lat"] = payload
                elif key == "lon":
                    message_building[id]["lon"] = payload
                elif key == "alt":
                    message_building[id]["alt"] = payload
            else:
                print("Missing lat, lon, or alt")

                if key == "lat":
                    message_building[id]["lat"] = payload
                elif key == "lon":
                    message_building[id]["lon"] = payload
                elif key == "alt":
                    message_building[id]["alt"] = payload
            return
        elif key == "spd":
            # convert speed to mph from knots
            payload = float(payload) * 1.15078
            message_building[id][key] = payload
        else:
            # add key and payload to message_building
            message_building[id][key] = payload
            # if telemetry data, send to mqtt
            if key not in nonTelemKeys:
                print("non-telemetry key: ", key)
                print(key, payload)
                # check if value is the same as last value
                if id in old_messages:
                    if key in old_messages[id] and payload == old_messages[id][key]:
                        return
                # send telemetry data to mqtt
                try:
                    topic = (
                        "TELEMETRY/"
                        + message_building[id]["name"]
                        + "-"
                        + str(message_building[id]["ssid"])
                    )
                    client.publish(
                        topic + "/" + key, json.dumps(payload), retain=True, qos=0
                    )
                except Exception as e:
                    print("Error sending telemetry to mqtt: ", e)
                try:
                    old_messages[id][key] = payload
                except:
                    pass
    else:
        print("Unknown topic: ", topic)


client.on_message = on_message

client.loop_forever()

# close mqtt connection
client.disconnect()
