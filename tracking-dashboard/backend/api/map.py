from utils.map import Map
from sql.helpers import get_items_last_n_minutes, get_positions_last_n_minutes

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for

map_app = Blueprint('map_app', __name__)

# TODO: add ability to periodically pull data/sync data to APRS-IS

# ideally, if a data point appears on APRS-IS for one of the callsigns we are listening for,
# we should add it to the map here and update our database accordingly

# we could also make it optionally possible for data sent to our endpoint to be pushed
# to APRS-IS so that it exists in both places and can still be accessed by other APRS clients

# in summary, we want to be able to allow our ground stations to have the options to:
# - upload just to APRS-IS and have this work as a client (both with and without a 
# sqlite database). Ideally this could be done from javascript as a pure frontend solution
# - upload just to our endpoint and have the option to push to (and pull from) APRS-IS
# - upload to both our endpoint and APRS-IS and have the option to push to (and pull from) APRS-IS

@map_app.route('/getTail', methods=['GET'])
def getTails():
    # get age param from request
    age = request.args.get('age') or 180
    age = int(age)
    # get name - is a string or a list of strings
    name = request.args.get('name')
    # turn name into a list if it is not already
    if name is not None:
        if not isinstance(name, list):
            name = [name]

    # should get all callsigns matching name (or all if None) that have transmitted in the last <age> minutes
    # default to 180 minutes
    # and return them as a list of dictionaries
    items = get_items_last_n_minutes(age, name)

    # now we have all active callsigns, we can get their locations over the last <age> minutes
    # and return them as a list of dictionaries
    positions = get_positions_last_n_minutes(age, name)
    print(positions)

    # return the data as a json object
    data = {}
    for item in items:
        name = f"{item.callsign}-{item.ssid}"
        data[name] = {
            "name": name,
            "symbol": item.symbol,
            "positions": []
        }
        for position in positions:
            if position["callsign"] == item.callsign and position["ssid"] == item.ssid:
                data[name]["positions"].append({
                    "lat": position["latitude"],
                    "lon": position["longitude"],
                    "alt": position["altitude"],
                    "cse": position["course"],
                    "cpd": position["speed"],
                    "cmnt": position["comment"],
                    "timestamp": position["timestamp"]
                })

    return jsonify(data)
