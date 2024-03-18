from xmlrpc.client import Boolean
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship, backref
from sqlalchemy import CheckConstraint

from sqlalchemy import Column, Integer, String, Text, DateTime, Float, ForeignKey
from sqlalchemy.dialects.postgresql import JSONB
from geoalchemy2 import Geometry
from sqlalchemy.sql import func

from datetime import datetime

Base = declarative_base()

class Message(Base):
    __tablename__ = 'messages'

    id = Column(Integer, primary_key=True)
    timestamp = Column(DateTime, server_default=func.now(), nullable=False)
    message = Column(Text, nullable=False, unique=True) # don't really want unique, but don't want to deal with duplicates right now

    def __repr__(self):
        return "<Message(id='%s', timestamp='%s', message='%s')>" % (self.id, self.timestamp, self.message)
    
class Source(Base):
    __tablename__ = 'sources'

    id = Column(Integer, primary_key=True)
    message = Column(Integer, ForeignKey('messages.id'), nullable=False)
    timestamp = Column(DateTime, nullable=False, default=datetime.utcnow)
    callsign = Column(String(6), nullable=False)
    ssid = Column(Integer, nullable=False)
    # TODO: change to inet
    ip = Column(String, nullable=False)
    signed = Column(Boolean, nullable=False, default=False)

    message_obj = relationship("Message", backref="sources")

    __table_args__ = (
        CheckConstraint(ssid >= 0, name='ssid_positive'),
        CheckConstraint(ssid <= 15, name='ssid_max_15'),
    )

    def __repr__(self):
        return "<Source(id='%s', message='%s', timestamp='%s', callsign='%s', ssid='%s', ip='%s')>" % (self.id, self.message, self.timestamp, self.callsign, self.ssid, self.ip)
    
class Telemetry(Base):
    __tablename__ = 'telemetry'

    id = Column(Integer, primary_key=True)
    message = Column(Integer, ForeignKey('messages.id'), nullable=False)
    raw = Column(Text, nullable=False)
    parsed = Column(JSONB, nullable=False)

    message_obj = relationship("Message", backref="telemetry")

    def __repr__(self):
        return "<Telemetry(id='%s', message='%s', raw='%s', parsed='%s')>" % (self.id, self.message, self.raw, self.parsed)
    
class Position(Base):
    __tablename__ = 'positions'

    id = Column(Integer, primary_key=True)
    message = Column(Integer, ForeignKey('messages.id'), nullable=False)
    callsign = Column(String(6), nullable=False)
    ssid = Column(Integer, nullable=False)
    symbol = Column(String(2), nullable=False)
    speed = Column(Float, nullable=True)
    course = Column(Float, nullable=True)
    geo = Column(Geometry('POINT'), nullable=True)
    altitude = Column(Float, nullable=True)
    comment = Column(String, nullable=True)
    telemetry = Column(Integer, ForeignKey('telemetry.id'), nullable=True)

    message_obj = relationship("Message", backref="positions")
    telemetry_obj = relationship("Telemetry", backref="positions")

    __table_args__ = (
        CheckConstraint(ssid >= 0, name='ssid_positive'),
        CheckConstraint(ssid <= 15, name='ssid_max_15'),
        CheckConstraint(course >= 0, name='course_positive'),
        CheckConstraint(course <= 360, name='course_max_360'),
    )

    def __repr__(self):
        return "<Position(id='%s', message='%s', callsign='%s', ssid='%s', symbol='%s', speed='%s', course='%s', geo='%s', altitude='%s', comment='%s', telemetry='%s')>" % (self.id, self.message, self.callsign, self.ssid, self.symbol, self.speed, self.course, self.geo, self.altitude, self.comment, self.telemetry)
