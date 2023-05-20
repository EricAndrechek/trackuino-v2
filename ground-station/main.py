from utilities import config, passcode
import sys


if __name__ == '__main__':
    config_file = sys.argv[1] if len(sys.argv) > 1 else 'config.yaml'
    conf = config.Config(config_file)
    passcode.add_passcode(conf)

    

    print(conf)
