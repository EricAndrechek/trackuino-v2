from utils.config_helper import config
from utils.map import Map

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for

map_app = Blueprint('map_app', __name__)

@map_app.route('/api/map', methods=['GET'])
def map_api():
    return "map api"