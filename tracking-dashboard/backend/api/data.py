import stat
from flask import Blueprint, request
import json

import symbol
from utils.api import Data
from sql.helpers import check_item_id, add_item_id

data_app = Blueprint('data_app', __name__)

# data upload endpoints

# upload should take data in the formats json and aprs, where data is replace with the aprs string or json object
example = {
    "callsign": "N0CALL",
    "ssid": 0,
    "timestamp": "2021-01-01T00:00:00Z",
    "type": "json",
    "data": {
        "callsign": "N0CALL",
        "ssid": 0,
        "symbol": "/",
        "lat": 0.0,
        "lon": 0.0,
        "alt": 0.0,
        "course": 0.0,
        "speed": 0.0,
        "comment": "test comment",
        "telemetry": {
            "battery": 0.0,
            "temperature": 0.0,
            "humidity": 0.0,
            "pressure": 0.0
        }
    }
}

# json upload for rock7 format
@data_app.route('/rock7-upload', methods=['POST'])
def api_rock7_upload():
    data_obj = Data()

    # receive data from request
    data = {}
    try:
        data = request.get_json()
    except Exception as e:
        # if data is not in json format, return error and show expected format with 400 status code
        # TODO: ideally prettier formatted json example?
        return "Data must be in json format. Example: " + str(example), 400
    
    # data here comes in format:
    '''
    {'momsn': 782, 'data': '48656c6c6f21205468697320697320612074657374206d6573736167652066726f6d20526f636b424c4f434b21', 'serial': 111111, 'iridium_latitude': 25.1694, 'iridium_cep': 52.0, 'JWT': 'really long jwt', 'imei': 'imeinumber', 'device_type': 'ROCKBLOCK', 'transmit_time': '24-03-29 03:20:26', 'iridium_longitude': 129.8162}
    '''
    # convert data['data'] to bytes from hex
    try:
        encoded_data = bytes.fromhex(data['data'])
        data['data'] = encoded_data.decode('utf-8')
        # json decode the data['data']
        decoded_data = json.loads(data['data'])
    except Exception as e:
        print("error decoding data: ", e)
        return "Data must be in json format. Example: " + str(example), 400

    # if callsign is not in data, add as serial
    if 'callsign' not in decoded_data:
        decoded_data['callsign'] = str(data['serial'])[:6]
    # if ssid is not in data, add as 0
    if 'ssid' not in decoded_data:
        decoded_data['ssid'] = 0
    # if timestamp is not in data, add as transmit_time
    if 'timestamp' not in decoded_data:
        decoded_data['timestamp'] = data['transmit_time']
    # if 'timestamp' is None or empty, add as transmit_time
    if decoded_data['timestamp'] is None or decoded_data['timestamp'] == "":
        decoded_data['timestamp'] = data['transmit_time']

    # set data to decoded_data
    source = {}
    source['callsign'] = "ROCK"
    source['ssid'] = 7
    source['timestamp'] = data['transmit_time']
    source['type'] = 'json'

    # check if id exists for callsign-ssid
    cs = data['callsign']
    ssid = data['ssid']
    symbol = "/O"
    rock7_id = data['serial']
    item = check_item_id(rock7_id)
    if item is None:
        add_item_id(rock7_id, cs, ssid, "\O")
    else:
        cs = item.callsign
        ssid = item.ssid
        symbol = item.symbol

    data = decoded_data
    source['data'] = data['data']
    source['data']['timestamp'] = decoded_data['timestamp']
    source['data']['callsign'] = cs
    source['data']['ssid'] = ssid
    source['data']['symbol'] = symbol

    
    # accept upload data
    try:
        data_obj.upload(source)
    except Exception as e:
        return str(e), 400
    
    # check if sender matches token
    data_obj.check_token(request.headers.get('Authorization'))

    # get ip address of client
    data_obj.get_client_ip(request)

    # parse data
    try:
        data_obj.parse()
    except Exception as e:
        return str(e), 400
    
    # save data
    try:
        status_code = data_obj.save()
        if status_code == 201 or status_code == 202:
            return "New data saved", 200
        elif status_code == 208:
            return "Data already exists", 200
    except Exception as e:
        print("save error: ", e)
        return str(e), 400

# JSON upload route
@data_app.route('/upload', methods=['POST'])
def api_upload():
    data_obj = Data()

    # receive data from request
    data = {}
    try:
        data = request.get_json()
    except Exception as e:
        # if data is not in json format, return error and show expected format with 400 status code
        # TODO: ideally prettier formatted json example?
        return "Data must be in json format. Example: " + str(example), 400
    
    # accept upload data
    try:
        data_obj.upload(data)
    except Exception as e:
        return str(e), 400
    
    # check if sender matches token
    data_obj.check_token(request.headers.get('Authorization'))

    # get ip address of client
    data_obj.get_client_ip(request)

    # parse data
    try:
        data_obj.parse()
    except Exception as e:
        return str(e), 400
    
    # save data
    try:
        status_code = data_obj.save()
        if status_code == 201 or status_code == 202:
            return "New data saved", status_code
        elif status_code == 208:
            return "Data already exists", 208
    except Exception as e:
        print("save error: ", e)
        return str(e), 400
