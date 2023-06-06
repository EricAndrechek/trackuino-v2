sudo apt update
sudo apt upgrade -y
sudo apt autoremove -y

echo ""
echo "[WARNING] If you have not finished setting up SSH access, please press CTRL+C and do so now."
echo "Once you have finished setting up SSH access, please reboot the server with the command 'sudo reboot' and run this script again."

read -p "Press enter once you have finished setting up SSH access and rebooted the server."




# -------------------- APT --------------------
# add postgres apt repo
sudo sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt $(lsb_release -cs)-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -

# add grafana apt repo
wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -
echo "deb https://packages.grafana.com/oss/deb stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list

# install libraries we are using
sudo apt update
sudo apt install postgresql postgresql-15-postgis-3 grafana python3-pip python3-venv python3-dev libpq-dev mosquitto mosquitto-clients redis-server nginx certbot python3-certbot-nginx -y


# -------------------- OPEN PORTS --------------------
# open ports for nginx
sudo iptables -I INPUT 6 -m state --state NEW -p tcp --dport 80 -j ACCEPT
sudo iptables -I INPUT 6 -m state --state NEW -p tcp --dport 443 -j ACCEPT
# open ports for mqtt
sudo iptables -I INPUT 6 -m state --state NEW -p tcp --dport 1883 -j ACCEPT
sudo netfilter-persistent save


# -------------------- NGINX SETUP --------------------
# setup nginx
# ask for root domain
read -p "What is your root domain? (e.g. example.com): " root_domain
# ask for mqtt domain
read -p "What is your mqtt domain? (e.g. mqtt.example.com): " mqtt_domain
# ask for grafana domain
read -p "What is your grafana domain? (e.g. status.example.com): " grafana_domain
# ask for api domain
read -p "What is your api domain? (e.g. api.example.com): " api_domain
# build nginx config for each domain
# TODO: allow mqtt to use wss and streams with tcp
sudo tee /etc/nginx/sites-available/$mqtt_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $mqtt_domain;
    return 301 https://\$host\$request_uri;
}
server {
    listen 443 ssl;
    listen [::]:443 ssl;
    server_name $mqtt_domain;
    ssl_certificate /etc/letsencrypt/live/$mqtt_domain/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/$mqtt_domain/privkey.pem;
    location / {
        proxy_pass http://localhost:9001;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
sudo tee /etc/nginx/sites-available/$grafana_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $grafana_domain;
    return 301 https://\$host\$request_uri;
}
server {
    listen 443 ssl;
    listen [::]:443 ssl;
    server_name $grafana_domain;
    ssl_certificate /etc/letsencrypt/live/$grafana_domain/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/$grafana_domain/privkey.pem;
    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
sudo tee /etc/nginx/sites-available/$api_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $api_domain;
    return 301 https://\$host\$request_uri;
}
server {
    listen 443 ssl;
    listen [::]:443 ssl;
    server_name $api_domain;
    ssl_certificate /etc/letsencrypt/live/$api_domain/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/$api_domain/privkey.pem;
    location / {
        proxy_pass http://localhost:8000;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
# enable nginx sites
sudo ln -s /etc/nginx/sites-available/$mqtt_domain /etc/nginx/sites-enabled/
sudo ln -s /etc/nginx/sites-available/$grafana_domain /etc/nginx/sites-enabled/
sudo ln -s /etc/nginx/sites-available/$api_domain /etc/nginx/sites-enabled/
# set default site to redirect to root domain
sudo rm /etc/nginx/sites-enabled/default
sudo tee /etc/nginx/sites-available/default > /dev/null <<EOT
server {
    listen 80 default_server;
    listen [::]:80 default_server;
    server_name _;
    return 301 https://$root_domain$request_uri;
}
EOT
sudo ln -s /etc/nginx/sites-available/default /etc/nginx/sites-enabled/

# ask for email for certbot
read -p "What is your email address? (for certbot): " email

# create certs
sudo certbot run -n --nginx --agree-tos -d $mqtt_domain,$grafana_domain,$api_domain --email $email --redirect


# -------------------- POSTGRES SETUP --------------------
# start postgres
sudo systemctl daemon-reload
sudo systemctl enable postgresql
sudo systemctl start postgresql

# setup postgres
cd ~postgres/
read -p "What would you like to set the postgres admin password to?: " password
sudo -u postgres psql -c "ALTER USER postgres PASSWORD '$password';"

# create grafana user
read -p "What would you like to set the postgres grafana password to?: " password
sudo -u postgres psql -c "CREATE USER grafana WITH PASSWORD '$password'; GRANT USAGE ON SCHEMA schema TO grafanareader; GRANT SELECT ON schema.table TO grafanareader;"

# create server user
read -p "What would you like to set the postgres server password to? (this is the one you will later add to the config): " password
sudo -u postgres psql -c "CREATE USER server WITH PASSWORD '$password'; ALTER USER server WITH CREATEDB;"
sudo -u postgres psql -c "CREATE DATABASE db OWNER server; GRANT ALL PRIVILEGES ON DATABASE db TO server;"

# create postgis extension for db
sudo -u postgres psql -d db -c "CREATE EXTENSION postgis;"


# -------------------- MOSQUITTO SETUP --------------------
# enable mosquitto
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# allow mosquitto anonymous access
sudo tee /etc/mosquitto/conf.d/default.conf > /dev/null <<EOT
listener 1883
allow_anonymous true
EOT
sudo systemctl restart mosquitto

# -------------------- REDIS SETUP --------------------
# setup redis
# modify redis config to use systemd
sudo sed -i 's/supervised no/supervised systemd/g' /etc/redis/redis.conf

sudo systemctl enable redis-server
sudo systemctl start redis-server


# -------------------- GRAFANA SETUP --------------------
# setup node_exporter
cd ~
wget https://github.com/prometheus/node_exporter/releases/download/v1.6.0/node_exporter-1.6.0.linux-arm64.tar.gz
tar xvfz node_exporter-1.6.0.linux-arm64.tar.gz
chmod +x node_exporter-1.6.0.linux-arm64/node_exporter
sudo mv node_exporter-1.6.0.linux-arm64/node_exporter /usr/local/bin/
rm -rf node_exporter-1.6.0.linux-arm64*
sudo useradd -rs /bin/false node_exporter

# setup node_exporter service
sudo tee /etc/systemd/system/node_exporter.service > /dev/null <<EOT
[Unit]
Description=Node Exporter
After=network.target

[Service]
User=node_exporter
Group=node_exporter
Type=simple
ExecStart=/usr/local/bin/node_exporter

[Install]
WantedBy=multi-user.target
EOT

sudo systemctl daemon-reload
sudo systemctl enable node_exporter
sudo systemctl start node_exporter

# install prometheus
wget https://github.com/prometheus/prometheus/releases/download/v2.37.8/prometheus-2.37.8.linux-arm64.tar.gz
tar xvfz prometheus-2.37.8.linux-arm64.tar.gz
chmod +x prometheus-2.37.8.linux-arm64/prometheus
chmod +x prometheus-2.37.8.linux-arm64/promtool
sudo mv prometheus-2.37.8.linux-arm64/prometheus /usr/local/bin/
sudo mv prometheus-2.37.8.linux-arm64/promtool /usr/local/bin/
sudo mkdir /etc/prometheus
sudo mkdir /var/lib/prometheus
sudo mv prometheus-2.37.8.linux-arm64/consoles /etc/prometheus
sudo mv prometheus-2.37.8.linux-arm64/console_libraries /etc/prometheus
rm -rf prometheus-2.37.8.linux-arm64*
sudo useradd -rs /bin/false prometheus

# setup prometheus config
sudo tee /etc/prometheus/prometheus.yml > /dev/null <<EOT
global:
  scrape_interval: 10s

scrape_configs:
  - job_name: 'prometheus_metrics'
    scrape_interval: 5s
    static_configs:
      - targets: ['localhost:9090']
  - job_name: 'node_exporter_metrics'
    scrape_interval: 5s
    static_configs:
      - targets: ['localhost:9100']
EOT
sudo chown -R prometheus: /etc/prometheus /var/lib/prometheus

# setup prometheus service
sudo tee /etc/systemd/system/prometheus.service > /dev/null <<EOT
[Unit]
Description=Prometheus
After=network.target

[Service]
User=prometheus
Group=prometheus
Type=simple
ExecStart=/usr/local/bin/prometheus \
    --config.file /etc/prometheus/prometheus.yml \
    --storage.tsdb.path /var/lib/prometheus/ \
    --web.console.templates=/etc/prometheus/consoles \
    --web.console.libraries=/etc/prometheus/console_libraries

[Install]
WantedBy=multi-user.target
EOT

sudo systemctl daemon-reload
sudo systemctl enable prometheus
sudo systemctl start prometheus

# enable grafana
sudo systemctl enable grafana-server
sudo systemctl start grafana-server

# setup grafana
# allow anonymous access
sudo sed -i 's/;allow_sign_up = true/allow_sign_up = false/g' /etc/grafana/grafana.ini
sudo sed -i 's/;allow_anonymous = false/allow_anonymous = true/g' /etc/grafana/grafana.ini
sudo systemctl restart grafana-server

# TODO: automate adding prometheus as a data source
# TODO: automate adding Postgres as a data source
# TODO: automate adding MQTT as a data source
# TODO: automate adding Redis as a data source
# TODO: automate adding nginx as a data source
# TODO: add gunicorn as a data source
# TODO: automate adding dashboards