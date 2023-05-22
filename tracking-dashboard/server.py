from flask import Flask, render_template, request, jsonify, redirect, url_for
from config_helper import load_config as conf
import requests
import subprocess
import logging

app = Flask(__name__)

config = conf('config.yaml')

def get_tunnel_status():
    # TODO: caching, error handling, and status logging for graphing

    url = 'https://api.cloudflare.com/client/v4/accounts/{}/cfd_tunnel?is_deleted=false'.format(config['CF-Tunnel-User-ID'])
    headers = {
        'Authorization': 'Bearer {}'.format(config['CF-Tunnel-API-Key'])
    }
    r = requests.get(url, headers=headers)

    data = r.json()

    # for each tunnel, get the tunnel details
    for tunnel in data['result']:
        url = 'https://api.cloudflare.com/client/v4/accounts/{}/cfd_tunnel/{}/configurations'.format(config['CF-Tunnel-User-ID'], tunnel['id'])
        r = requests.get(url, headers=headers)
        tunnel['details'] = r.json()['result']['config']
    
    return data

@app.route('/api', methods=['POST'])
def api():
    data = request.get_json()
    return jsonify(data)

@app.route('/api/status', methods=['POST'])
def server_status():
    return "OK"

@app.route('/api/pull', methods=['GET'])
def api_pull():
    # pull the latest version of the repo and return the result
    result = subprocess.run(['git', 'pull'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return jsonify({'result': result.stdout.decode('utf-8')})

@app.route('/api/tunnel', methods=['GET'])
def api_tunnel():
    return jsonify(get_tunnel_status())

@app.route('/netdata/<path:filename>', methods=['GET'])
def netdata(filename):
    return redirect("https://{}/{}".format(config['Netdata-Host'], filename), code=302)

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
    logging.basicConfig(filename='error.log',level=logging.DEBUG)
    app.run(debug=True)