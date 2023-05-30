from utils.config_helper import config

import requests
import json
import humanfriendly

class MetricParser:
    # takes in data from netdata api and returns it with each metric as a key
    def __init__(self, data, metrics):
        self.data = data
        self.metrics = metrics
        self.result = {}
    
    def get_val(self, chart, dimension):
        return self.data[chart]['dimensions'][dimension]['value']
    
    def get_units(self, chart):
        return self.data[chart]['units']
    
    def cpu(self):
        self.result['cpu'] = "{}%".format(round(100 - self.get_val('system.cpu', 'idle'), 2))
    
    def perc_ram(self):
        self.result['perc_ram'] = "{}%".format(round(self.get_val('system.ram', 'used') / (self.get_val('system.ram', 'free') + self.get_val('system.ram', 'cached') + self.get_val('system.ram', 'buffers') + self.get_val('system.ram', 'used')) * 100, 2))

    def used_ram(self):
        og_ram_str = "{} {}".format(self.get_val('system.ram', 'used'), self.get_units('system.ram'))
        parsed_ram_str = humanfriendly.parse_size(og_ram_str)
        self.result['used_ram'] = "{}".format(humanfriendly.format_size(parsed_ram_str))
    
    def uptime(self):
        og_uptime_str = "{} {}".format(self.get_val('system.uptime', 'uptime'), self.get_units('system.uptime'))
        parsed_uptime_str = humanfriendly.parse_timespan(og_uptime_str)
        self.result['uptime'] = "{}".format(humanfriendly.format_timespan(parsed_uptime_str))
    
    def traffic_up(self):
        unit_size = self.get_units('system.net').split('/')[0]
        og_traffic_up_str = "{} {}".format(abs(self.get_val('system.net', 'OutOctets')), unit_size)
        parsed_traffic_up_str = humanfriendly.parse_size(og_traffic_up_str)
        self.result['traffic_up'] = "{}".format(humanfriendly.format_size(parsed_traffic_up_str))
    
    def traffic_down(self):
        unit_size = self.get_units('system.net').split('/')[0]
        og_traffic_down_str = "{} {}".format(abs(self.get_val('system.net', 'InOctets')), unit_size)
        parsed_traffic_down_str = humanfriendly.parse_size(og_traffic_down_str)
        self.result['traffic_down'] = "{}".format(humanfriendly.format_size(parsed_traffic_down_str))
    
    def traffic_total(self):
        unit_size = self.get_units('system.net').split('/')[0]
        og_traffic_total_str = "{} {}".format(abs(self.get_val('system.net', 'OutOctets')) + abs(self.get_val('system.net', 'InOctets')), unit_size)
        parsed_traffic_total_str = humanfriendly.parse_size(og_traffic_total_str)
        self.result['traffic_total'] = "{}".format(humanfriendly.format_size(parsed_traffic_total_str))
    
    def io(self):
        unit_size = self.get_units('system.io').split('/')[0]
        og_io_str = "{} {}".format(abs(self.get_val('system.io', 'in')) + abs(self.get_val('system.io', 'out')), unit_size)
        parsed_io_str = humanfriendly.parse_size(og_io_str)
        self.result['io'] = "{}".format(humanfriendly.format_size(parsed_io_str))
    
    def perc_disk(self):
        self.result['perc_disk'] = "{}%".format(round(self.get_val('disk_space._', 'used') / (self.get_val('disk_space._', 'used') + self.get_val('disk_space._', 'avail')) * 100, 2))

    def used_disk(self):
        og_disk_str = "{} {}".format(self.get_val('disk_space._', 'used'), self.get_units('disk_space._'))
        parsed_disk_str = humanfriendly.parse_size(og_disk_str)
        self.result['used_disk'] = "{}".format(humanfriendly.format_size(parsed_disk_str))
    
    def free_disk(self):
        og_disk_str = "{} {}".format(self.get_val('disk_space._', 'avail'), self.get_units('disk_space._'))
        parsed_disk_str = humanfriendly.parse_size(og_disk_str)
        self.result['free_disk'] = "{}".format(humanfriendly.format_size(parsed_disk_str))

    def parse(self):
        # run functions for each metric
        for metric in self.metrics:
            getattr(self, metric.replace(' ', '_'))()
        
        return self.result



class ND_Stats:
    # parse netdata api data according to: https://raw.githubusercontent.com/netdata/netdata/master/web/api/netdata-swagger.yaml
    def __init__(self):
        self.host = config.status.netdata.host

        self.nodes = self._get_nodes()

        self.metrics = config.status.netdata.metrics

        self._get_all_stats()
    
    def _get_nodes(self):
        # returns list of nodes in format:
        # [
        #     {
        #         "id": "1234567890abcdef1234567890abcdef",
        #         "name": "node1",
        #         "status": "healthy"
        #     },
        #     {
        #         "id": "1234567890abcdef1234567890abcdef",
        #         "name": "node2",
        #         "status": "offline"
        #     }
        # ]

        url = self.host + "/api/v2/nodes"
        r = requests.get(url)

        data = r.json()
        
        # only keep matching keys
        # lots more keys available, but these are the only ones officially documented
        data['nodes'] = [{key: node[key] for key in ['nm', 'ni', 'state'] if key in node} for node in data['nodes']]

        # replace state key values
        for node in data['nodes']:
            if node['state'] == 'reachable':
                node['state'] = 'healthy'
            elif node['state'] == 'stale':
                node['state'] = 'inactive'
            elif node['state'] == 'unreachable':
                node['state'] = 'offline'
            else:
                raise Exception("Unknown node state: {}".format(node['state']))

        return data['nodes']

    def _get_node_metrics(self, node):
        # modify node object to include metrics

        # if node is offline, don't bother
        if node['state'] != 'healthy':
            return node
        
        url = self.host
        # if node is host (ni = 0), don't include nm in url
        if node['ni'] != 0:
            url += "/host/" + node['nm']
        url += "/api/v1/allmetrics?format=json"

        r = requests.get(url)

        data = r.json()

        # parse data
        parser = MetricParser(data, self.metrics)
        node['metrics'] = parser.parse()
    
        return node
    
    def _get_all_stats(self):
        # get stats for all nodes
        for node in self.nodes:
            self._get_node_metrics(node)
    
    def update(self):
        # update stats for all nodes
        self._get_all_stats()
    
    def refresh(self):
        # refresh nodes list
        self.nodes = self._get_nodes()
        self.update()
        

def main():
    nd = ND_Stats()
    print(json.dumps(nd.nodes, indent=4))
    
