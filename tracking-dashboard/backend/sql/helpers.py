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