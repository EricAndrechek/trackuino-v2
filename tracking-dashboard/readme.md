# Tracking Dashboard

This is an open-source application to either pull data from APRS-IS or from your own database and API endpoint for tracking your balloons and vehicles.

This requires a server to host this webpage and its database. You can also host the webpage with any other static hosting provider.

This tool is cloud agnostic, although I highly recommend [Oracle's "always free" tier](https://www.oracle.com/cloud/free/) as it has the needed compute and database storage for everything we need to do.

# Stack

This application is rather complicated. It has:

- Nginx to handle, secure, route incoming connections, and (optionally) serve the frontend
- CertBot to handle SSL certs
- a Flask server to glue each component together and act as an API
- a Redis queue to handle async workers for longer jobs, like parsing APRS data and predicting flight paths
- a Python Redis Queue worker to run the longer jobs
- a PostgreSQL database to store position packets
- a Mosquitto MQTT broker for pub/sub of position packets
- Grafana for displaying server stats and telemetry and data from each client
- Node Exporter for exposing OS metrics to Prometheus
- Prometheus for sending OS metrics to Grafana

# Setup

## Setup VMs

Go to [Oracle's "always free" tier](https://www.oracle.com/cloud/free/) and create an account. Now create a compute instance as the ARM type and max out the specs you are allowed. See the `server.tf` file for the automated deployment script. Add your ssh key now. Set it up **without** an IP so that you can assign it a permanent IP from the virtual network or VNIC's settings page.

## Clone Repo

```bash
git clone --no-checkout --depth=1 --filter=tree:0 https://github.com/EricAndrechek/trackuino-v2.git
cd trackuino-v2
git sparse-checkout set --no-cone tracking-dashboard
git checkout
cd tracking-dashboard
chmod +x setup.sh
./setup.sh
```

## Setup Server

Now you have an Ubuntu server with a static IP address that you can SSH into. We now need to setup the server for everything it requires. Most of this process is automated via the setup script, but we will explain what is going on here too.
