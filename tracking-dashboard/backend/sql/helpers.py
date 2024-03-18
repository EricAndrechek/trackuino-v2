try:
    from sql.db import Session
    from sql.models import *
except ImportError:
    import sys
    sys.path.append("..")
    from sql.db import Session
    from sql.models import *
from sqlalchemy.exc import IntegrityError
from psycopg2.errors import UniqueViolation
from sqlalchemy import cast, func

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
            data.append(message.message)
    except Exception as e:
        print(e)
    return data

def bulk_add_messages(messages, request):
    for message in messages:
        data_obj = Data()
        try:
            data_obj.upload(message)
        except Exception as e:
            return e, 400
        data_obj.check_token(request.headers.get('Authorization'))

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
        except Exception as e:
            print("save error: ", e)
            return e, 400
    return "New data saved", 201

# given an ip address, get the last time that ip address added a source
def get_last_ip_addition(ip):
    time = None
    try:
        time = Session.query(func.max(Source.timestamp)).filter_by(ip=ip).first()[0]
    except Exception as e:
        print(e)
    
    return time
