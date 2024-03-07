from box import Box, ConfigBox, exceptions

try:
    config = ConfigBox(Box.from_yaml(filename='config.yaml', camel_killer_box=True))
except exceptions.BoxError:
    config = ConfigBox(Box.from_yaml(filename='../config.yaml', camel_killer_box=True))