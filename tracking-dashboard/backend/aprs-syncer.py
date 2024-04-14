import aprslib
from sql.helpers import check_item_id, add_item_id, get_all_callsigns
from utils.api import Data

# ----- APRS SETUP -----

# get all callsigns in items database
callsigns = get_all_callsigns()
# get just first item from each tuple
callsigns = [callsign[0] for callsign in callsigns]
# just unique callsigns
callsigns = list(set(callsigns))
# create filter string
filter = "p/" + "/".join(callsigns)
print(filter)

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

    # check if callsign exists in items table
    print("Data: ", data_obj.data)
    name = data_obj.data['callsign'] + "-" + str(data_obj.data['ssid'])
    item = check_item_id(name)
    if item is None:
        # add item to items table
        add_item_id(name, data_obj.data['callsign'], data_obj.data['ssid'], data_obj.data['symbol'])
    else:
        # change callsign, name, and symbol to item values
        data_obj.data['callsign'] = item.callsign
        data_obj.data['ssid'] = item.ssid
        data_obj.data['symbol'] = item.symbol
    
    # save data
    try:
        status_code = data_obj.save()
        if status_code == 201 or status_code == 202:
            print("New data saved")
        elif status_code == 208:
            print("Data already exists")
    except Exception as e:
        print("save error: ", e)

# connect to APRS-IS and listen for all callsigns in items database
aprs = aprslib.IS("N0CALL", port=14580)
aprs.set_filter(filter)
aprs.connect()
aprs.consumer(callback)
