from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for
from config_helper import config
import requests
import subprocess
import logging
import json

app = Flask(__name__)

# custom endpoints

from cloudflare import cf_app
from netdata import nd_app
app.register_blueprint(cf_app)
app.register_blueprint(nd_app)

# api endpoints

@app.route('/api', methods=['POST'])
def api():
    data = request.get_json()
    return jsonify(data)

@app.route('/api/status', methods=['POST'])
def server_status():
    return "OK"

@app.route('/api/pull', methods=['GET'])
def api_pull():
    # TODO: set to work only from specific IP addresses or with a token
    # so that it can't be abused (ie only allow GH actions to trigger it)

    # TODO: figure out how to make it work when run under production WSGI and systemd

    # pull the latest version of the repo and return the result
    result = subprocess.run(['git', 'pull'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return jsonify({'result': result.stdout.decode('utf-8')})

# static pages

@app.route('/', defaults={'path': 'index'})
@app.route('/<path:path>')
def show(path):
    try:
        return render_template(path + '.html', title=config.title)
    except Exception as e:
        return render_template('404.html', title=config.title), 404

# error handlers
    
@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html', title=config.title), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('500.html', title=config.title, msg=e)

# main

if __name__ == '__main__':
    if config.debug:
        print("Config:")
        print(json.dumps(config, indent=4))
    logging.basicConfig(filename='error.log', level=logging.DEBUG if config.debug else logging.INFO)
    app.run(debug=True if config.debug else False, host=config.host, port=config.port)