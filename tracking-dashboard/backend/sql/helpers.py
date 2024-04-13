try:
    from sql.db import Session
    from sql.models import *
except ImportError:
    import sys
    sys.path.append("..")
    from sql.db import Session
    from sql.models import *
from datetime import timedelta
from re import L
from sqlalchemy.exc import IntegrityError
from psycopg2.errors import UniqueViolation
from sqlalchemy import cast, func
from tasks.mqttadder import add_data

# add new datum to mqtt (position or telemetry)
def add_datum(name, data, timestamp=None):
    # add to redis queue for adding to mqtt by worker
    if isinstance(data, Position):
        mqtt_data = {
            "name": name,
            "lat": data.latitude,
            "lon": data.longitude,
            "alt": data.altitude,
            "cse": data.course,
            "spd": data.speed,
            "cmnt": data.comment,
            "sym": data.symbol,
            "dt": timestamp
        }
        print("adding position to mqtt")
        add_data("position", mqtt_data)
    elif isinstance(data, Telemetry):
        mqtt_data = {
            "name": name,
            "telemetry": data.parsed
        }
        print("adding telemetry to mqtt")
        add_data("telemetry", mqtt_data)


# takes a message object and adds it to the database
# returns message id of message object and whether or not it was added
def add_message(message):
    Session.add(message)
    try:
        Session.commit()
        return (True, message.id)
    except IntegrityError as e:
        Session.rollback()
        Session.flush()
        print(e)
    
    # if message already exists, return id of existing message
    return (False, Session.query(Message).filter_by(message=message.message).first().id)

def add_source(source):
    Session.add(source)
    try:
        Session.commit()
        return (True, source.id)
    except IntegrityError as e:
        print(e)
        Session.rollback()
        Session.flush()
    
    # if source already exists, return id of existing source
    return (False, Session.query(Source).filter_by(callsign=source.callsign).first().id)

def add_position(position):
    Session.add(position)
    try:
        Session.commit()
        return (True, position.id)
    except IntegrityError as e:
        print(e)
        Session.rollback()
        Session.flush()
    
    # if position already exists, return id of existing position
    return (False, Session.query(Position).filter_by(callsign=position.callsign).first().id)

def add_telemetry(telemetry):
    Session.add(telemetry)
    try:
        Session.commit()
        return (True, telemetry.id)
    except IntegrityError as e:
        print(e)
        Session.rollback()
        Session.flush()
    
    # if telemetry already exists, return id of existing telemetry
    return (False, Session.query(Telemetry).filter_by(battery=telemetry.battery).first().id)

from utils.api import Data

# get all messages since a given timestamp
def get_since_timestamp(timestamp):
    # print("Getting all new messages since: ", timestamp)
    data = []
    try:
        messages = Session.query(Message).filter(Message.timestamp > timestamp).all()
        for message in messages:
            print(message)
            data.append({"timestamp": message.timestamp, "message": message.message})
    except Exception as e:
        print(e)
    return data

# get all messages since given callsign last added a source
def get_since_callsign(callsign):
    # print("Getting all new messages since callsign: ", callsign)
    data = []
    try:
        timestamp = Session.query(func.max(Source.timestamp)).filter_by(callsign=callsign).first()[0]
        messages = Session.query(Message).filter(Message.timestamp > timestamp).all()
        for message in messages:
            data.append({"timestamp": message.timestamp, "message": message.message})
    except Exception as e:
        print(e)
    return data

def bulk_add_messages(messages, request):
    if not isinstance(messages, list):
        messages = [messages]
    for message in messages:
        data_obj = Data()
        try:
            data_obj.upload(message)
        except Exception as e:
            return str(e), 400
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
            status_code = data_obj.save(True)
        except Exception as e:
            print("bulk save error: ", e)
            return str(e), 400
    return "New data saved", 201

# given an ip address, get the last time that ip address added a source
def get_last_ip_addition(ip):
    time = None
    try:
        time = Session.query(func.max(Source.timestamp)).filter_by(ip=ip).first()[0]
    except Exception as e:
        print(e)
    
    return time

# get all items in the items table that have been updated in the last n minutes
# should match all names (callsign-ssid) in table, or all names in a list of names - if names is none, return all items
def get_items_last_n_minutes(n, name):
    items = []
    try:
        if name is None:
            items = Session.query(Items).filter(Items.last_updated > datetime.utcnow() - timedelta(minutes=n)).all()
        else:
            items = Session.query(Items).filter(Items.last_updated > datetime.utcnow() - timedelta(minutes=n)).filter(Items.callsign.in_(name)).all()
    except Exception as e:
        print(e)
    return items

# get position data over the last n minutes for all given callsigns+ssid pairs
def get_positions_last_n_minutes(n, names):
    messages = []
    # start by getting all message ids for the given callsigns and ssids within the last n minutes from the Message table
    try:
        if names is None:
            messages = Session.query(Message).filter(Message.timestamp > datetime.utcnow() - timedelta(minutes=n)).all()
        else:
            messages = Session.query(Message).filter(Message.timestamp > datetime.utcnow() - timedelta(minutes=n)).filter(Message.callsign.in_(names)).all()
    except Exception as e:
        print(e)
        return []
    
    positions = []
    # get all positions for the given message ids and add their message timestamp
    try:
        for message in messages:
            new_positions = Session.query(Position).filter_by(message=message.id).all()
            for position in new_positions:
                positions.append({
                    "timestamp": message.timestamp,
                    "callsign": position.callsign,
                    "ssid": position.ssid,
                    "symbol": position.symbol,
                    "speed": position.speed,
                    "course": position.course,
                    "latitude": position.latitude,
                    "longitude": position.longitude,
                    "altitude": position.altitude,
                    "comment": position.comment
                })
    except Exception as e:
        print(e)
    return positions

# check if an id is in the items table, returning the callsign, ssid, and symbol if it is - and updating the last_updated time to now
def check_item_id(id):
    item = None
    try:
        item = Session.query(Items).filter_by(id=id).first()
        if item is not None:
            item.last_updated = datetime.utcnow()
            Session.commit()
    except Exception as e:
        print(e)
    return item


# add a given id with optional callsign, ssid, and symbol to the items table
def add_item_id(id, callsign=None, ssid=None, symbol=None):
    item = Items(id=id, callsign=callsign, ssid=ssid, symbol=symbol)
    Session.add(item)
    try:
        Session.commit()
        return True
    except Exception as e:
        print(e)
        Session.rollback()
        Session.flush()
    return False

# get all callsigns in the items table
def get_all_callsigns():
    callsigns = []
    try:
        callsigns = Session.query(Items.callsign).distinct().all()
    except Exception as e:
        print(e)
    return callsigns
