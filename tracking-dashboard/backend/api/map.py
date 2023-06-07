from utils.map import Map

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

@map_app.route('/map', methods=['GET'])
def map_api():
    return "map api"