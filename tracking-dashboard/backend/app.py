import gevent
import gevent.monkey

gevent.monkey.patch_all()

import psycogreen.gevent

psycogreen.gevent.patch_psycopg()

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for
from utils.config_helper import config
from utils.pydb import PyDB
import requests
import subprocess
import logging
import json

app = Flask(__name__)

# API ENDPOINTS

from api import api_bp

app.register_blueprint(api_bp, url_prefix='/api')

# ERROR HANDLERS
    
@app.errorhandler(404)
def page_not_found(e):
    return "Not found", 404

@app.errorhandler(500)
def internal_server_error(e):
    return "Internal server error: {}".format(e), 500

# MAIN

if __name__ == '__main__':
    if config.debug:
        print("Config:")
        print(json.dumps(config, indent=4))
    logging.basicConfig(filename='error.log', level=logging.DEBUG if config.debug else logging.INFO)
    app.run(debug=True if config.debug else False, host=config.host, port=config.port)