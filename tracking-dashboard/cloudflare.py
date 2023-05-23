# cloudflare specific api endpoints and code

from server import app, cf_config
import requests
from flask import Flask, render_template, request, jsonify, redirect, url_for

def get_tunnel_status():
    # TODO: caching, error handling, and status logging for graphing

    url = 'https://api.cloudflare.com/client/v4/accounts/{}/cfd_tunnel?is_deleted=false'.format(cf_config['CF-Tunnel-User-ID'])
    headers = {
        'Authorization': 'Bearer {}'.format(cf_config['CF-Tunnel-API-Key'])
    }
    r = requests.get(url, headers=headers)

    data = r.json()

    # for each tunnel, get the tunnel details
    for tunnel in data['result']:
        url = 'https://api.cloudflare.com/client/v4/accounts/{}/cfd_tunnel/{}/configurations'.format(cf_config['CF-Tunnel-User-ID'], tunnel['id'])
        r = requests.get(url, headers=headers)
        tunnel['details'] = r.json()['result']['config']
    
    return data

@app.route('/api/tunnel', methods=['GET'])
def api_tunnel():
    return jsonify(get_tunnel_status())

