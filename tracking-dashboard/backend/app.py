import gevent
import gevent.monkey

gevent.monkey.patch_all()

import psycogreen.gevent

psycogreen.gevent.patch_psycopg()

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for
from flask_cors import CORS
from utils.config_helper import config

from sql.db import Session

app = Flask(__name__)
CORS(app)

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
