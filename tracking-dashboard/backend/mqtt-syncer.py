# constant running python script to sync data from mqtt to postgres

import paho.mqtt.client as mqtt
from datetime import datetime
from sql.helpers import check_item_id, add_item_id
from utils.api import Data

# open mqtt connection
client = mqtt.Client()
client.connect("localhost", 1883, 60)

# subscribe to all trackers
client.subscribe("T/#")

message_building = {}

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
        message['callsign'] = message_building[id]['name']
    else:
        message['callsign'] = str(id)[:6]
    if "ssid" in message_building[id]:
        message['ssid'] = message_building[id]['ssid']
    else:
        message['ssid'] = 0
    if "sym" in message_building[id]:
        message['symbol'] = message_building[id]['sym']
    else:
        message['symbol'] = "/>"
    if "lat" in message_building[id]:
        message['lat'] = message_building[id]['lat']
    else:
        message['lat'] = None
    if "lon" in message_building[id]:
        message['lon'] = message_building[id]['lon']
    else:
        message['lon'] = None
    if "alt" in message_building[id]:
        message['alt'] = message_building[id]['alt']
    else:
        message['alt'] = None
    if "cse" in message_building[id]:
        message['course'] = message_building[id]['cse']
    else:
        message['course'] = None
    if "spd" in message_building[id]:
        message['speed'] = message_building[id]['spd']
    else:
        message['speed'] = None
    if "hh" in message_building[id] and "mm" in message_building[id] and "ss" in message_building[id] and "YY" in message_building[id] and "MM" in message_building[id] and "DD" in message_building[id]:
        message['timestamp'] = f"{message_building[id]['YY']}-{message_building[id]['MM']}-{message_building[id]['DD']} {message_building[id]['hh']}:{message_building[id]['mm']}:{message_building[id]['ss']}"
    else:
        message['timestamp'] = None
    
    # for all other keys, add them to telemetry
    telemetry = {}
    for key in message_building[id]:
        if key not in ['name', 'ssid', 'sym', 'lat', 'lon', 'alt', 'cse', 'spd', 'hh', 'mm', 'ss', 'YY', 'MM', 'DD']:
            telemetry[key] = message_building[id][key]
    
    if len(telemetry) > 0:
        message['telemetry'] = telemetry
    
    return message
    

# build a source packet of the type the API likes from a message
def build_source_packet(message):
    source = {}
    source['timestamp'] = str(datetime.now().isoformat())
    source['callsign'] = "UMSERV"
    source['ssid'] = 0
    source['ip'] = "127.0.0.1"
    source['type'] = 'json'
    source['data'] = message

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
                    if "ssis" in message_building[id]:
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
                message_building[id]['name'] = item.callsign
                message_building[id]['ssid'] = item.ssid
                message_building[id]['sym'] = item.symbol
        
        # periodically check that name, ssid, and symbol haven't been overwritten in the items table
        # if they have, update message_building
        if key == "ss" and id in message_building and (payload == "00" or payload == "15" or payload == "30" or payload == "45" or payload == "60"):
            item = check_item_id(id)
            if item is not None:
                message_building[id]['name'] = item.callsign
                message_building[id]['ssid'] = item.ssid
                message_building[id]['sym'] = item.symbol
            else:
                print("Item not found in items table: ", id)
                return


        # if key is "ss" (seconds), add message to db
        if key == "ss":
            # add message to db
            msg = build_json_message(id)
            if msg is None:
                return
            src = build_source_packet(msg)
            data_obj = Data()
            message_building[id]['ss'] = timestamp
            try:
                print("Uploading data: ", src)
                data_obj.upload(src)
            except Exception as e:
                print("Error uploading data: ", e)
                return
            try:
                data_obj.parse()
            except Exception as e:
                print("Error parsing data: ", e)
                return
            try:
                status_code = data_obj.save()
                if status_code == 201:
                    print("New data saved")
                elif status_code == 208:
                    print("Data already exists")
            except Exception as e:
                print("save error: ", e)
            return
        else:
            # add key and payload to message_building
            message_building[id][key] = payload
    else:
        print("Unknown topic: ", topic)


client.on_message = on_message

client.loop_forever()

# close mqtt connection
client.disconnect()
