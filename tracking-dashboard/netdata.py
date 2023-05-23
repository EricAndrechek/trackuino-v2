from server import app, nd_config
from flask import Flask, render_template, request, jsonify, redirect, url_for

@app.route('/netdata/<path:filename>', methods=['GET'])
def netdata(filename):
    # TODO: maybe change to a 301 redirect when not in debug mode
    return redirect("{}/{}".format(nd_config['Netdata-Host'], filename), code=302)