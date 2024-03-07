# enable mosquitto
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# allow mosquitto anonymous access
sudo tee /etc/mosquitto/conf.d/default.conf > /dev/null <<EOT
listener 1883
allow_anonymous true
EOT
sudo systemctl restart mosquitto