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

        self.info = {
            "callsign": None,
            "ssid": None,
            "type": None,
            "timestamp": None,
            "ip": None,
        }

        self.data = {}

        # try parsing data object
        try:
            self.info["type"] = data["type"]

            if "from" in data:
                self.info["callsign"] = data["from"].split("-")[0]
                try:
                    self.info["ssid"] = data["from"].split("-")[1]
                except:
                    self.info["ssid"] = 0
            else:
                self.info["callsign"] = data["callsign"]
                self.info["ssid"] = data["ssid"]

        except KeyError:
            raise Exception("No callsign or ssid in upload data")
        
        try:
            self.info["timestamp"] = data["timestamp"]
        except KeyError:
            self.info["timestamp"] = None
    
    def check_token(self, token):
        # check aprs callsign and token with passcode util
        if token is not None:
            if get_passcode(self.info["callsign"]) == token:
                self.signed = True
            else:
                # TODO: log invalid token
                # probably also want to inform user that token is invalid
                self.signed = False

    
    def parse_aprs(self):
        # parse aprs data

        self.data = {
            "callsign": None,
            "ssid": None,
            "symbol": None,
            "lat": None,
            "lon": None,
            "alt": None,
            "course": None,
            "speed": None,
            "comment": None,
        }

        # parse aprs data
        # check for required fields
        try:
            callsign = self.info["callsign"]
            ssid = self.info["ssid"]
            try:
                ssid = int(ssid)
            except ValueError:
                raise Exception("Invalid ssid")
            symbol = self.raw["symbol_table"] + self.raw["symbol"]
            if callsign is None or ssid is None or symbol is None:
                print(callsign, ssid, symbol)
                raise KeyError
            if len(callsign) > 6:
                raise Exception("Callsign too long")
            if ssid < 0 or ssid > 15:
                raise Exception("Invalid ssid")
            if len(symbol) > 2:
                raise Exception("Symbol too long")
            self.data["callsign"] = callsign
            self.data["ssid"] = ssid
            self.data["symbol"] = symbol
        except KeyError:
            raise Exception("Parse APRS: Missing required fields in upload data")
        
        lat = None
        try:
            lat = self.raw["latitude"]
            try:
                lat = float(lat)
            except ValueError:
                raise Exception("Invalid latitude")
            if lat < -90 or lat > 90:
                raise Exception("Invalid latitude")
        except KeyError:
            pass

        lon = None
        try:
            lon = self.raw["longitude"]
            try:
                lon = float(lon)
            except ValueError:
                raise Exception("Invalid longitude")
            if lon < -180 or lon > 180:
                raise Exception("Invalid longitude")
        except KeyError:
            pass

        alt = None
        try:
            alt = self.raw["altitude"]
        except KeyError:
            pass

        course = None
        try:
            course = self.raw["course"]
            try:
                course = float(course) 
            except ValueError:
                raise Exception("Invalid course")
            if course < 0 or course > 360:
                raise Exception("Invalid course")
        except KeyError:
            pass

        speed = None
        try:
            speed = self.raw["speed"]
            try:
                # convert speed in knots to mph
                speed = float(speed) * 1.15078
            except ValueError:
                raise Exception("Invalid speed")
        except KeyError:
            pass

        comment = None
        try:
            comment = self.raw["comment"]
        except KeyError:
            pass

        self.data["lat"] = lat
        self.data["lon"] = lon
        self.data["alt"] = alt
        self.data["course"] = course
        self.data["speed"] = speed
        self.data["comment"] = comment

        # TODO: APRS telemetry data

    def parse_json(self):
        # parse json data
        self.data = self.raw["data"]

        # check for required fields
        # verify callsign, ssid, symbol exists and are valid
        try:
            callsign = self.raw["data"]["callsign"]
            ssid = self.raw["data"]["ssid"]
            try:
                ssid = int(ssid)
            except ValueError:
                raise Exception("Invalid ssid")
            symbol = self.raw["data"]["symbol"]
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
            lat = self.raw["data"]["lat"]
            try:
                lat = float(lat)
            except ValueError:
                raise Exception("Invalid latitude")
            if lat < -90 or lat > 90:
                raise Exception("Invalid latitude")
        except KeyError:
            pass

        lon = None
        try:
            lon = self.raw["data"]["lon"]
            try:
                lon = float(lon)
            except ValueError:
                raise Exception("Invalid longitude")
            if lon < -180 or lon > 180:
                raise Exception("Invalid longitude")
        except KeyError:
            pass

        alt = None
        try:
            alt = self.raw["data"]["alt"]
        except KeyError:
            pass

        course = None
        try:
            course = self.raw["data"]["course"]
            try:
                course = float(course)
            except ValueError:
                raise Exception("Invalid course")
            if course < 0 or course > 360:
                raise Exception("Invalid course")
        except KeyError:
            pass

        speed = None
        try:
            speed = self.raw["data"]["speed"]
        except KeyError:
            pass

        comment = None
        try:
            comment = self.raw["data"]["comment"]
        except KeyError:
            pass

        self.data["callsign"] = callsign
        self.data["ssid"] = ssid
        self.data["symbol"] = symbol
        self.data["lat"] = lat
        self.data["lon"] = lon
        self.data["alt"] = alt
        self.data["course"] = course
        self.data["speed"] = speed
        self.data["comment"] = comment
        if "telemetry" in self.raw["data"]:
            self.data["telemetry"] = self.raw["data"]["telemetry"]

    def parse_data(self):
        # check if data is in aprs format or json format
        if self.info["type"] == "aprs":
            # possible for uploading raw APRS data, but JSON is ideal?
            self.parse_aprs()
        elif self.info["type"] == "json":
            self.parse_json()
        else:
            raise Exception("Invalid type in upload data")

        self.parse()
    
    def parse(self):
        # check if data is in aprs format or json format
        if self.info["type"] == "aprs":
            print("Is aprs")
            self.data["raw"] = self.raw["raw"]
        elif self.info["type"] == "json":
            self.data["raw"] = json.dumps(self.raw["data"])
        else:
            raise Exception("Invalid type in upload data")
    
    def save(self, save_timestamp=False):
        to_return = 400

        message = Message(
            message=self.data["raw"],
            timestamp=self.info["timestamp"] if save_timestamp else None,
        )

        print(message)

        first, message_id = add_message(message)

        print(first, message_id)

        if first:
            self.parse_data()

            print("parsed data")

            telemetry_success, telemetry_id = (False, None)
            if "telemetry" in self.data:
                print("telemetry exists")

                # add to telemetry table if telemetry data exists

                # TODO: this is data duplication and could be fixed
                telemetry = Telemetry(
                    message=message_id,
                    raw=self.data["raw"],
                    parsed=self.data["telemetry"],
                )
                # telemetry_success, telemetry_id = add_telemetry(telemetry)

                # add to mqtt
                name = self.data["callsign"] + "-" + str(self.data["ssid"])
                add_datum(name, telemetry)

                # print(telemetry_success, telemetry_id)
            
            # if position data exists, add to positions table
            if 'lat' in self.data and 'lon' in self.data:
                print("position exists")

                # add to position table
                position = Position(
                    callsign=self.data["callsign"],
                    ssid=self.data["ssid"],
                    symbol=self.data["symbol"],
                    latitude=self.data["lat"],
                    longitude=self.data["lon"],
                    altitude=self.data["alt"],
                    course=self.data["course"],
                    speed=self.data["speed"],
                    comment=self.data["comment"],
                    telemetry=telemetry_id if telemetry_success else None,
                    message=message_id,
                )

                print(position)

                position_success, position_id = add_position(position)

                print(position_success, position_id)

                # add to mqtt
                name = self.data["callsign"] + "-" + str(self.data["ssid"])
                add_datum(name, position, str(message.timestamp))

                to_return = 201
            else:
                # only a telemetry packet
                to_return = 202
        else:
            to_return = 208

        print(to_return)
        
        # save to sources table with message id

        source = Source(
            callsign=self.info["callsign"],
            ssid=self.info["ssid"],
            message=message_id,
            ip=self.info["ip"],
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
        
        self.info["ip"] = ip if ip is not None else "unknown"
