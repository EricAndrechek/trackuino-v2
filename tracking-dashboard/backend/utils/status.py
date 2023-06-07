import json

from utils.config_helper import config

class Status:
    def __init__(self):
        self.status = {}
            

def main():
    status = Status()
    print(json.dumps(status.status, indent=4))