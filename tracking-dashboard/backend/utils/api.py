from utils.config_helper import config
import json

from sql.models import *
from sql.helpers import *

class Data:
    def __init__(self):
        pass

    def upload(self, data):
        self.raw = data["data"]

        self.data = {}

        # try parsing data object
        try:
            self.data["callsign"] = data["callsign"]
            self.data["ssid"] = data["ssid"]
            self.data["type"] = data["type"]
        except KeyError:
            raise Exception("No callsign or ssid in upload data")
        
        try:
            self.data["timestamp"] = data["timestamp"]
        except KeyError:
            self.data["timestamp"] = None
    
    def check_token(self, token):
        # do aprs token check here
        return True
    
    def parse_aprs(self):
        # parse aprs data
        pass

    def parse_json(self):
        # parse json data
        self.data["data"] = self.raw["data"]

    def parse_data(self):
        # check if data is in aprs format or json format
        if self.data["type"] == "aprs":
            self.parse_aprs()
        elif self.data["type"] == "json":
            self.parse_json()
        else:
            raise Exception("Invalid type in upload data")

        # check for required fields
        # verify callsign, ssid, symbol exists and are valid
        try:
            callsign = self.data["data"]["callsign"]
            ssid = self.data["data"]["ssid"]
            symbol = self.data["data"]["symbol"]
            if callsign is None or ssid is None or symbol is None:
                raise KeyError
            if len(callsign) > 6:
                raise Exception("Callsign too long")
            if ssid < 0 or ssid > 15:
                raise Exception("Invalid ssid")
            if len(symbol) > 2:
                raise Exception("Symbol too long")
        except KeyError:
            raise Exception("Missing required fields in upload data")
    
    def parse(self):
        # check if data is in aprs format or json format
        if self.data["type"] == "aprs":
            self.data["raw"] = self.raw
        elif self.data["type"] == "json":
            self.data["raw"] = json.dumps(self.raw)
        else:
            raise Exception("Invalid type in upload data")
    
    def save(self):
        to_return = 400

        message = Message(
            message=self.data["raw"],
        )

        print(message)

        first, message_id = add_message(message)

        print(first, message_id)

        if first:
            self.parse_data()
            # TODO: save to telemetry and data tables with message id
            to_return = 201
        else:
            to_return = 208

        print(to_return)
        
        # save to sources table with message id

        source = Source(
            callsign=self.data["callsign"],
            ssid=self.data["ssid"],
            message=message_id,
            ip=self.data["ip"],
        )

        print(source)
        
        success, source_id = add_source(source)

        print(success, source_id)

        return to_return

    def get_client_ip(self, request):
        ip = request.headers.get('X-Forwarded-For')
        if ip is None:
            ip = request.remote_addr
        
        self.data["ip"] = ip if ip is not None else "unknown"
