from utils.config_helper import config

import requests
import json

class CF_Tunnels:
    # API parsing based on: https://raw.githubusercontent.com/cloudflare/api-schemas/main/openapi.yaml
    def __init__(self):
        self.user_id = config.status.cloudflare.user_id
        self.api_key = config.status.cloudflare.api_key

        self.headers = {
            'Authorization': 'Bearer {}'.format(self.api_key)
        }

        self.show_deleted = config.status.cloudflare.show_deleted
        self.show_hostnames = config.status.cloudflare.show_hostnames

        self.tunnels = self._get_tunnels()
        self._get_all_tunnel_data(self.tunnels)

    def _get_tunnels(self):
        # returns list of tunnels in format:
        # [
        #     {
        #         "id": "1234567890abcdef1234567890abcdef",
        #         "name": "tunnel1",
        #         "connections": 1,
        #         "status": "healthy"
        #     },
        #     {
        #         "id": "1234567890abcdef1234567890abcdef",
        #         "name": "tunnel2",
        #         "connections": 0,
        #         "status": "offline"
        #     }
        # ]

        url = 'https://api.cloudflare.com/client/v4/accounts/{}/cfd_tunnel?is_deleted={}'.format(self.user_id, str(self.show_deleted).lower())
        r = requests.get(url, headers=self.headers)

        data = r.json()

        if data['success'] == False:
            raise Exception(data['errors'][0]['message'])
        
        # only keep matching keys
        data['result'] = [{key: tunnel[key] for key in ['id', 'name', 'connections', 'status'] if key in tunnel} for tunnel in data['result']]

        # change connections key to length of connections list
        for tunnel in data['result']:
            tunnel['connections'] = len(tunnel['connections'])

        return data['result']

    def _get_tunnel_hosts(self, tunnel_id):
        # returns tunnel hosts in format:
        # {
        #     "tunnel_id": "1234567890abcdef1234567890abcdef",
        #     "hosts": [
        #         {
        #             "hostname": "example.com",
        #             "service": "http://localhost:5000",
        #         },
        #         {
        #             "hostname": "example.com/other",
        #             "service": "http://localhost:8080",
        #         }
        #     ]
        # }
        if self.show_hostnames:
            url = 'https://api.cloudflare.com/client/v4/accounts/{}/cfd_tunnel/{}/configurations'.format(self.user_id, tunnel_id)
            r = requests.get(url, headers=self.headers)
            
            data = r.json()

            if data['success'] == False:
                raise Exception(data['errors'][0]['message'])
            
            # only keep data.result.tunnel_id and data.result.config.ingress
            data['result'] = {key: data['result'][key] for key in ['tunnel_id', 'config'] if key in data['result']}
            data['result']['config'] = {key: data['result']['config'][key] for key in ['ingress'] if key in data['result']['config']}

            # only keep data.result.config.ingress[].hostname, data.result.config.ingress[].service, and data.result.config.ingress[].path
            data['result']['hosts'] = [{key: ingress[key] for key in ['hostname', 'service', 'path'] if key in ingress} for ingress in data['result']['config']['ingress']]
            # if path exists, append it to hostname
            # if no hostname exists, remove the host
            for host in data['result']['hosts']:
                if 'path' in host:
                    host['hostname'] += host['path']
                    del host['path']
                if 'hostname' not in host:
                    data['result']['hosts'].remove(host)
            
            del data['result']['config']

            return data['result']
        else:
            return {
                "tunnel_id": tunnel_id,
                "hosts": []
            }
    
    def _get_all_tunnel_data(self, tunnels):
        # modifies tunnels to include hosts
        for tunnel in tunnels:
            tunnel['hosts'] = self._get_tunnel_hosts(tunnel['id'])['hosts']
    
    def update(self):
        # tunnel healths
        self.tunnels = self._get_tunnels()
        # grab tunnel hosts
        self._get_all_tunnel_data(self.tunnels)
    
    def refresh(self):
        # refresh doesn't do anything different from update for cloudflare
        self.update()

def main():
    tunnels = CF_Tunnels()
    print(json.dumps(tunnels.tunnels, indent=4))