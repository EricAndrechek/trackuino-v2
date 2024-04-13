# connect to APRS-IS and listen for all callsigns in items database, mirroring their data to our database

import aprslib
from sql.helpers import get_all_callsigns
from utils.api import Data

def callback(packet):
    data_obj = Data()

    # receive data from request
    data = packet
    data["type"] = "aprs"

    print("Received data: ", data)
    
    # accept upload data
    try:
        data_obj.upload(data)
    except Exception as e:
        print("upload error: ", e)
    
    data_obj.info['ip'] = '127.0.0.1'

    print("About to parse")

    # parse data
    try:
        data_obj.parse()
    except Exception as e:
        print("parse error: ", e)
    
    # save data
    try:
        status_code = data_obj.save()
        if status_code == 201 or status_code == 202:
            print("New data saved")
        elif status_code == 208:
            print("Data already exists")
    except Exception as e:
        print("save error: ", e)

# get all callsigns in items database
callsigns = get_all_callsigns()
# get just first item from each tuple
callsigns = [callsign[0] for callsign in callsigns]
# just unique callsigns
callsigns = list(set(callsigns))
callsigns.append("KF8ABL")
callsigns.append("W8AXP")
callsigns.append("N9FEB")
callsigns.append("KB6OJE")
callsigns.append("KJ7MLW")
callsigns.append("KJ6ZOY")
callsigns.append("N4KCB")
callsigns.append("DK3MM")
callsigns.append("ZBPDT")
callsigns.append("JH0EYA")
callsigns.append("KC7RUF")
callsigns.append("VE7IKX")
# create filter string
filter = "p/" + "/".join(callsigns)
print(filter)

# connect to APRS-IS and listen for all callsigns in items database
aprs = aprslib.IS("N0CALL", port=14580)
aprs.set_filter(filter)
aprs.connect()
aprs.consumer(callback)
