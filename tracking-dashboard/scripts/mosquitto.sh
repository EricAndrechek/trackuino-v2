# enable mosquitto
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# allow mosquitto anonymous access
sudo tee /etc/mosquitto/mosquitto.conf > /dev/null <<EOT
listener 1883
allow_anonymous true

listener 1884
protocol websockets
EOT
sudo systemctl restart mosquitto
