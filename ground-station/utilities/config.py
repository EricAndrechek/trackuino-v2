from box import Box, ConfigBox, exceptions

# TODO: check all values to verify they are correct

try:
    config = ConfigBox(Box.from_yaml(filename='config.yaml', camel_killer_box=True))
except exceptions.BoxError:
    config = ConfigBox(Box.from_yaml(filename='../config.yaml', camel_killer_box=True))