# setup redis
# modify redis config to use systemd
sudo sed -i 's/supervised no/supervised systemd/g' /etc/redis/redis.conf

sudo systemctl enable redis-server
sudo systemctl start redis-server