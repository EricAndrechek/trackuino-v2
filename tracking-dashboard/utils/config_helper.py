from box import Box, ConfigBox

config = ConfigBox(Box.from_yaml(filename='config.yaml', camel_killer_box=True))