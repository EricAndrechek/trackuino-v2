from flask import Flask, render_template, request, jsonify
from config_helper import load_config as conf
import requests
import subprocess

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

@app.route('/api/pull', methods=['GET'])
def api_pull():
    # pull the latest version of the repo and return the result
    result = subprocess.run(['git', 'pull'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return jsonify({'result': result.stdout.decode('utf-8')})

@app.route('/api/tunnel', methods=['GET'])
def api_tunnel():
    return jsonify(get_tunnel_status())

@app.route('/', defaults={'path': 'index'})
@app.route('/<path:path>')
def show(path):
    return render_template(path + '.html', title=config['Title'])

if __name__ == '__main__':
    print(config)
    app.run(debug=True)