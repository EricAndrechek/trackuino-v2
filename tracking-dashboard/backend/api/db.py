from logging import log
from flask import Blueprint, request, jsonify
from sql.helpers import bulk_add_messages, get_last_ip_addition, get_since_timestamp, get_since_callsign

from datetime import datetime
import json

db_app = Blueprint('db_app', __name__)

# database endpoints

# send a utc iso timestamp to the server, and get all data from the db since that timestamp or update the db with new data since that timestamp
@db_app.route('/sync', methods=['GET', 'POST'])
def api_db_sync():
    if request.method == 'GET':
        # check if timestamp is in the request, if not, set as now
        timestamp = request.args.get('timestamp')
        callsign = request.args.get('callsign')
        if callsign is None:
            if timestamp is None:
                # get all entries since last time this ip added to sources table, limit to 1000
                ip = None
                try:
                    ip = request.environ['HTTP_X_REAL_IP']
                except:
                    ip = None
                if ip is None:
                    ip = request.headers.get('X-Forwarded-For')
                    if ip is None:
                        ip = request.remote_addr
                try:
                    timestamp = get_last_ip_addition(ip)
                    if timestamp is None:
                        return "No previous timestamp found for ip.", 400
                    # convert to utc
                    timestamp = timestamp.astimezone().replace(tzinfo=None)
                except Exception as e:
                    return str(e), 400
            else:
                # convert to datetime object
                try:
                    timestamp = datetime.fromisoformat(timestamp)
                    timestamp = timestamp.astimezone().replace(tzinfo=None)
                except ValueError as e:
                    return "Timestamp must be in iso format (YYYY-MM-DDTHH:MM:SSZ).", 400
            # get all data from the db since the timestamp
            data = get_since_timestamp(timestamp)
            if len(data) == 0:
                return "No new data found.", 204
            for i in range(len(data)):
                data[i] = json.loads(data[i])
            return jsonify(data), 200
        else:
            # get all entries since last time this callsign added to sources table, limit to 1000
            data = get_since_callsign(callsign)
            if len(data) == 0:
                return "No new data found.", 204
            for i in range(len(data)):
                data[i] = json.loads(data[i])
            return jsonify(data), 200
    elif request.method == 'POST':
        # update the db with new data
        data = {}
        try:
            data = request.get_json()
        except Exception as e:
            # if data is not in json format, return error and show expected format with 400 status code
            return "Data must be a list of messages in json format.", 400
        
        return bulk_add_messages(data, request)

@db_app.route('/db', methods=['DELETE'])
def api_db_delete():
    # call the delete method of the db class
    # TODO: implement delete method (need to figure out context of db class) <-- I don't remember what I meant when I wrote this...
    # TODO: need authorization
    return "not implemented", 501

@db_app.route('/db', methods=['GET'])
def api_db_get():
    # get all data from the db and return the files as a downloadable zip
    # TODO: need to implement, probably need to run as a background job...
    return "not implemented", 501

# TODO: allow getting specific tables from the db and render them in the browser
# maybe just redirect to pgadmin instead?
