# Automatically loads config file and returns a dictionary

import yaml

def Config(config_file):
    with open(config_file) as f:
        config = yaml.load(f, Loader=yaml.FullLoader)
    return config

# TODO: add a function to change and save the config file, potentially from the web interface
