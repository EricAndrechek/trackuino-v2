from flask import Blueprint, request

from utils.api import Data

data_app = Blueprint('data_app', __name__)

# data upload endpoints

# upload should take data in the formats json and aprs, where data is replace with the aprs string or json object
# {
#     "callsign": "N0CALL",
#     "ssid": 0,
#     "timestamp": "2021-01-01T00:00:00Z",
#     "type": "json",
#     "data": {
#         "callsign": "N0CALL",
#         "ssid": 0,
#         "symbol": "/",
#         "lat": 0.0,
#         "lon": 0.0,
#         "alt": 0.0,
#         "course": 0.0,
#         "speed": 0.0,
#         "comment": "test comment"
#         "telemetry": {
#             "battery": 0.0,
#             "temperature": 0.0,
#             "humidity": 0.0,
#             "pressure": 0.0
#         }
#     }
# }

@data_app.route('/upload', methods=['POST'])
def api_upload():
    data_obj = Data()

    # receive data from request
    data = {}
    try:
        data = request.get_json()
    except Exception as e:
        return "invalid json", 400
    
    # accept upload data
    try:
        data_obj.upload(request)
    except Exception as e:
        return e, 400
    
    # check if sender matches token
    if not data_obj.check_token(request.headers.get('Authorization')):
        return "invalid token", 401

    # get ip address of client
    data_obj.get_client_ip(request)

    # parse data
    try:
        data_obj.parse()
    except Exception as e:
        return e, 400
    
    # save data
    try:
        status_code = data_obj.save()
        if status_code == 201:
            return "New data saved", 201
        elif status_code == 208:
            return "Data already exists", 208
    except Exception as e:
        return e, 400