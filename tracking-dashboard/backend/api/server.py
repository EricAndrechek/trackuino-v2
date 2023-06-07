from utils.config_helper import config
import json
import subprocess
import datetime

from flask import Flask, Blueprint, render_template, request, jsonify, redirect, url_for

server_app = Blueprint('server_app', __name__)

# API ENDPOINTS

# pull the latest version of the repo from github
@server_app.route('/pull', methods=['GET'])
def api_pull():
    # ensure that the auth header is present and correct bearer token
    if 'Authorization' not in request.headers or request.headers.get('Authorization') != ('Bearer ' + config.push_key):
        return jsonify({'error': 'invalid authorization header'}), 401
    
    # TODO: navigate to the correct directory before running git pull

    # run git pull and send the output back to the client
    pull_result = subprocess.run(['git', 'pull'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    # ensure that the pull was successful
    if pull_result.returncode != 0:
        return jsonify({'pull': pull_result.stdout.decode('utf-8')})
    
    # enter the venv and inst all new dependencies
    # activate the venv
    activate_result = subprocess.run(['source', 'venv/bin/activate'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    # install any new dependencies
    install_result = subprocess.run(['pip3', 'install', '-r', 'requirements.txt'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    # return the output of both commands
    return jsonify({'pull': pull_result.stdout.decode('utf-8'), 'install': install_result.stdout.decode('utf-8')})


@server_app.route('/health', methods=['GET'])
def api_health():
    # return current date and time
    to_return = {
        'status': 'ok',
        'time': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    }
    return jsonify(to_return)