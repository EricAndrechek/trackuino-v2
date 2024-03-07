import helpers
from models import *

message = Message(
    message="test message"
)

print(helpers.add_message(message))

source = Source(
    callsign="test",
    ssid=0,
    message=1,
    ip="192.168.0.1"
)

print(helpers.add_source(source))