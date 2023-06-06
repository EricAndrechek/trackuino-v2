import gevent
import gevent.monkey

gevent.monkey.patch_all()

import psycogreen.gevent

psycogreen.gevent.patch_psycopg()

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for
from utils.config_helper import config
from utils.pydb import PyDB
from utils.api import Data
import requests
import subprocess
import logging
import json

app = Flask(__name__)

@app.context_processor
def inject_user():
    return dict(title=config.title, map_enabled=config.map.enabled, status_enabled=config.status.enabled)

# CUSTOM ENDPOINTS

# status page api endpoint

if config.status.enabled:
    from blueprints.status import status_app
    app.register_blueprint(status_app)

# map page api endpoint

if config.map.enabled:
    from blueprints.map import map_app
    app.register_blueprint(map_app)

# DEFAULT ENDPOINT DISABLE LOGIC

# should flow as follows:
# if map is enabled, show map
# if map is disabled, if status is enabled, show status
# if map is disabled, if status is disabled, show 404

if config.map.enabled:
    @app.route('/', methods=['GET'])
    def index():
        return render_template('map.html')
    @app.route('/map', methods=['GET'])
    def map():
        return render_template('map.html')
else:
    @app.route('/map', methods=['GET'])
    def map():
        return render_template('404.html'), 404

if config.status.enabled:
    if not config.map.enabled:
        @app.route('/', methods=['GET'])
        def index():
            return render_template('status.html')
    @app.route('/status', methods=['GET'])
    def status():
        return render_template('status.html')
else:
    @app.route('/status', methods=['GET'])
    def status():
        return render_template('404.html'), 404


# API ENDPOINTS

# TODO: set to work only from specific IP addresses or with a token
# so that they can't be abused (ie only allow GH actions to trigger it)

# force all cached http requests to refresh (also happens on restart)
@app.route('/api/refresh', methods=['POST'])
def cache_refresh():
    # should run the .refresh() method of status (likely will want to make it async)
    return "OK"

# pull the latest version of the repo from github
@app.route('/api/pull', methods=['GET'])
def api_pull():
    # TODO: figure out how to make it work when run under production WSGI and systemd

    # TODO: needs to install any new dependencies from requirements.txt

    # pull the latest version of the repo and return the result
    result = subprocess.run(['git', 'pull'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return jsonify({'result': result.stdout.decode('utf-8')})


@app.route('/api/health', methods=['GET'])
def api_health():
    return jsonify({'result': 'OK'})

# database endpoints

@app.route('/api/db', methods=['DELETE'])
def api_db_delete():
    # TODO: call the delete method of the db class (need to figure out context of db class)
    return "not implemented", 501

@app.route('/api/db', methods=['GET'])
def api_db_get():
    # TODO: get the current db file and return it
    return "not implemented", 501

# TODO: allow getting specific tables from the db and render them in the browser

# data upload endpoints

# upload should take data in the formats json and aprs, where data is replace with the aprs string or json object
# {
#     "callsign": "N0CALL",
#     "ssid": 0,
#     "timestamp": "2021-01-01T00:00:00Z",
#     "type": "json",
#     "data": {
#         "callsign": "N0CALL",
#         "ssid": 0,
#         "symbol": "/",
#         "lat": 0.0,
#         "lon": 0.0,
#         "alt": 0.0,
#         "course": 0.0,
#         "speed": 0.0,
#         "comment": "test comment"
#         "telemetry": {
#             "battery": 0.0,
#             "temperature": 0.0,
#             "humidity": 0.0,
#             "pressure": 0.0
#         }
#     }
# }

@app.route('/api/upload', methods=['POST'])
def api_upload():
    data_obj = Data()

    # receive data from request
    data = {}
    try:
        data = request.get_json()
    except Exception as e:
        return "invalid json", 400
    
    # accept upload data
    try:
        data_obj.upload(request)
    except Exception as e:
        return e, 400
    
    # check if sender matches token
    if not data_obj.check_token(request.headers.get('Authorization')):
        return "invalid token", 401

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
        if status_code == 201:
            return "New data saved", 201
        elif status_code == 208:
            return "Data already exists", 208
    except Exception as e:
        return e, 400


# CATCH ALL FOR HTML PAGES THAT AREN'T EXPLICITLY DEFINED

@app.route('/', defaults={'path': 'index'})
@app.route('/<path:path>')
def show(path):
    try:
        return render_template(path + '.html')
    except Exception as e:
        return render_template('404.html'), 404

# ERROR HANDLERS
    
@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html'), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html', msg=e)

# MAIN

if __name__ == '__main__':
    if config.debug:
        print("Config:")
        print(json.dumps(config, indent=4))
    logging.basicConfig(filename='error.log', level=logging.DEBUG if config.debug else logging.INFO)
    app.run(debug=True if config.debug else False, host=config.host, port=config.port)