from config_helper import config

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for

nd_app = Blueprint('nd_app', __name__)

@nd_app.route('/netdata/<path:filename>', methods=['GET'])
def netdata(filename):
    # TODO: maybe change to a 301 redirect when not in debug mode
    return redirect("{}/{}".format(config['Netdata-Host'], filename), code=302)