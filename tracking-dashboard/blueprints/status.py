from utils.config_helper import config
from utils.status import Status

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for

status_app = Blueprint('status_app', __name__)

# TODO: need some sort of job scheduler to update status every ~5 seconds

@status_app.route('/api/status', methods=['GET'])
def status_api():
    # for now, we do Status().status every time
    # once we have a job scheduler, we can just return the .status value and not redefine Status() every time
    return jsonify(Status().status)