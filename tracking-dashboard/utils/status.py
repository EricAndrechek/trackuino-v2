import json

from utils.config_helper import config

if config.status.cloudflare.enabled:
    from utils.cloudflare import CF_Tunnels

if config.status.netdata.enabled:
    from utils.netdata import ND_Stats

from utils.pydb import PyDB

class Status:
    def __init__(self):
        if config.status.cloudflare.enabled:
            self.cf = CF_Tunnels()

        if config.status.netdata.enabled:
            self.nd = ND_Stats()
        
        self.status = []

        self._get_status()

        # TODO: integrate with database to store status history and make sure references are up to date
    
    def _package_host_data(self, cf=None, nd=None):
        # verify that at least one is not None
        if cf is None and nd is None:
            raise Exception('Both cf and nd are None')
        
        package = {
            'name': cf['name'] if cf is not None else nd['nm'],
            'status': cf['status'] if cf is not None else nd['state'],
            'metrics': [],
            'hosts': []
        }

        if nd is not None:
            package['metrics'] = nd['metrics']

        if cf is not None:
            for host in cf['hosts']:
                if not host['service'].startswith('ssh://') or not config.status.cloudflare.hide_ssh_hostnames:
                    package['hosts'].append({
                        'hostname': host['hostname'],
                        'service': host['service'].split('://')[0]
                    })
            package['metrics']["connections"] = cf['connections']
        
        return package
    
    def _search_tunnels(self, key, value):
        if 'name' in key:
            for tunnel in self.cf.tunnels:
                if tunnel['name'] == value:
                    return tunnel
        elif 'id' in key:
            for tunnel in self.cf.tunnels:
                if tunnel['id'] == value:
                    return tunnel
        
        raise Exception('Tunnel not found')
    
    def _search_nodes(self, key, value):
        if 'name' in key:
            for node in self.nd.nodes:
                if node['nm'] == value:
                    return node
        elif 'id' in key:
            for node in self.nd.nodes:
                if node['ni'] == value:
                    return node
        
        raise Exception('Node not found')
    
    def _get_status(self):
        # go through each host defined in config
        for host in config.status.hosts:
            tunnel = None
            node = None

            # if host is a cloudflare tunnel
            cf_key = [key for key in host if 'cf' in key]
            if len(cf_key) > 0:
                # search for tunnel by name or id
                tunnel = self._search_tunnels(cf_key[0], host[cf_key[0]])
            
            # if host is a netdata node
            nd_key = [key for key in host if 'nd' in key]
            if len(nd_key) > 0:
                # search for node by name or id
                node = self._search_nodes(nd_key[0], host[nd_key[0]])
            
            # create a package for the host
            package = self._package_host_data(cf=tunnel, nd=node)

            # add package to status
            self.status.append(package)
    
    def update(self):
        if config.status.cloudflare.enabled:
            self.cf.update()
        
        if config.status.netdata.enabled:
            self.nd.update()
    
    def refresh(self):
        if config.status.cloudflare.enabled:
            self.cf.refresh()
        
        if config.status.netdata.enabled:
            self.nd.refresh()
            

def main():
    status = Status()
    print(json.dumps(status.status, indent=4))