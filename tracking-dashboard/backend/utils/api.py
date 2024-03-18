from utils.config_helper import config
from utils.passcode import get_passcode
import json

from sql.models import *
from sql.helpers import *

class Data:
    def __init__(self):
        self.raw = None
        self.data = None
        self.signed = False

    def upload(self, data):
        try:
            self.raw = data
        except KeyError:
            raise Exception("No data in upload data")

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
        # check aprs callsign and token with passcode util
        if token is not None:
            if get_passcode(self.data["callsign"]) == token:
                self.signed = True
            else:
                # TODO: log invalid token
                # probably also want to inform user that token is invalid
                self.signed = False

    
    def parse_aprs(self):
        # parse aprs data

        # TODO: need to implement - will be the same implemenation as in ground station

        # this should only be run when importing data from APRS-IS servers
        # ground stations, etc, should parse and send data in json format
        pass

    def parse_json(self):
        # parse json data
        self.data["data"] = self.raw["data"]

        # should we have assertions to check for required fields?
        # this is done in the save function

    def parse_data(self):
        # check if data is in aprs format or json format
        if self.data["type"] == "aprs":
            # possible for uploading raw APRS data, but JSON is ideal?
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
        
        lat = None
        try:
            lat = self.data["data"]["lat"]
            if lat < -90 or lat > 90:
                raise Exception("Invalid latitude")
        except KeyError:
            pass

        lon = None
        try:
            lon = self.data["data"]["lon"]
            if lon < -180 or lon > 180:
                raise Exception("Invalid longitude")
        except KeyError:
            pass

        alt = None
        try:
            alt = self.data["data"]["alt"]
        except KeyError:
            pass

        course = None
        try:
            course = self.data["data"]["course"]
            if course < 0 or course > 360:
                raise Exception("Invalid course")
        except KeyError:
            pass

        speed = None
        try:
            speed = self.data["data"]["speed"]
        except KeyError:
            pass

        comment = None
        try:
            comment = self.data["data"]["comment"]
        except KeyError:
            pass

        self.data["data"]["lat"] = lat
        self.data["data"]["lon"] = lon
        self.data["data"]["alt"] = alt
        self.data["data"]["course"] = course
        self.data["data"]["speed"] = speed
        self.data["data"]["comment"] = comment
    
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

            print("parsed data")

            telemetry_success, telemetry_id = (False, None)
            if "telemetry" in self.data["data"]:
                # add to telemetry table if telemetry data exists
                telemetry = Telemetry(
                    message=message_id,
                    raw=self.data["raw"],
                    parsed=self.data["data"]["telemetry"],
                )
                telemetry_success, telemetry_id = add_telemetry(telemetry)

                print(telemetry_success, telemetry_id)
            
            # build Geometry POINT object for lat/lon
            # format: 'POINT(-33.9034 152.73457)'
            geo_point = f"POINT({self.data['data']['lat']} {self.data['data']['lon']})" if "lat" in self.data["data"] and self.data["data"]["lat"] is not None and "lon" in self.data["data"] and self.data["data"]["lon"] is not None else None

            # add to position table
            position = Position(
                callsign=self.data["callsign"],
                ssid=self.data["ssid"],
                symbol=self.data["data"]["symbol"],
                geo=geo_point,
                altitude=self.data["data"]["alt"],
                course=self.data["data"]["course"],
                speed=self.data["data"]["speed"],
                comment=self.data["data"]["comment"],
                telemetry=telemetry_id if telemetry_success else None,
                message=message_id,
            )
            position_success, position_id = add_position(position)

            print(position_success, position_id)

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
            signed=self.signed
        )

        print(source)
        
        success, source_id = add_source(source)

        print(success, source_id)

        return to_return

    def get_client_ip(self, request):
        # try nginx's passed real ip first
        ip = None
        try:
            ip = request.environ['HTTP_X_REAL_IP']
        except:
            ip = None
        if ip is None:
            ip = request.headers.get('X-Forwarded-For')
            if ip is None:
                ip = request.remote_addr
        
        # only allow valid ip addresses
        # remove invalid characters (ipv4 and ipv6)
        ip = "".join([c for c in ip if c in "0123456789abcdef.:"])
        
        self.data["ip"] = ip if ip is not None else "unknown"
