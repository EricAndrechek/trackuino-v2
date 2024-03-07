# check that running arm64
if [ $(uname -m) != "aarch64" ]; then
    echo "Grafana, node_exporter, and prometheus auto setup has only been built for arm64"
else
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
fi