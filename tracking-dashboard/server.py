from flask import Flask, render_template, request, jsonify, redirect, url_for
from config_helper import load_config as conf
import requests
import subprocess
import logging

app = Flask(__name__)

config = conf('config.yaml')

cf_config = config['Cloudflare']
nd_config = config['Netdata']

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

@app.route('/', defaults={'path': 'index'})
@app.route('/<path:path>')
def show(path):
    try:
        return render_template(path + '.html', title=config['Title'])
    except:
        return render_template('404.html', title=config['Title'])
    
@app.errorhandler(404)
def page_not_found(e):
    return render_template('404.html', title=config['Title']), 404


if __name__ == '__main__':
    print(config)
    logging.basicConfig(filename='error.log', level=logging.DEBUG if config['Debug'] else logging.INFO)
    app.run(debug=True if config['Debug'] else False, host=config['Host'], port=config['Port'])