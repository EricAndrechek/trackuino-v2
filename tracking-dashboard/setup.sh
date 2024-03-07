echo "Beginning setup..."

# find the directory this script is in
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $DIR

sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get autoremove -y

echo ""
echo "[WARNING] If you have not finished setting up SSH access, please press CTRL+C and do so now."
echo "Once you have finished setting up SSH access, please reboot the server with the command 'sudo reboot' and run this script again."
echo "Additionally, please note that if you have changed settings from the defaults this script initializes things as, they will be overwritten by running this script."

read -p "Press enter once you have finished setting up SSH access and rebooted the server."

# -------------------- ASK WHAT TO INSTALL --------------------
echo "You will now be asked what you would like to install. Please answer y or n to each question."
echo "All of the following are optional, and you can run this script again to install any of them later."
echo "In order to properly run the backend, everything except the frontend must be installed, or configured manually to be run on another machine."
echo ""
echo "FRONTEND SERVICES: The frontend web interface the user interacts with."
read -p "Would you like to setup the frontend web interface? (y/n): " frontend_setup
echo ""
echo "BACKEND SERVICES: The microservices that do the heavy lifting to make the frontend work."
read -p "Would you like to setup the postgres database? (y/n): " postgres_setup
read -p "Would you like to setup the mosquitto message broker? (y/n): " mosquitto_setup
read -p "Would you like to setup the redis cache and background worker? (y/n): " redis_setup
read -p "Would you like to setup the backend web API? (y/n): " backend_setup
read -p "Would you like to setup the grafana data visualizer? (y/n): " grafana_setup
read -p "Would you like to setup nginx, the load balancer and reverse proxy? (y/n): " nginx_setup


# -------------------- APT --------------------
echo "Setting up custom APT repositories..."
# libaries list:
libraries=""

while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
curl
git
build-essential
libssl-dev
libffi-dev
python3-dev
python3-setuptools
python3-pip
python3-venv
EOF
)

if [ $frontend_setup = "y" ]; then
    # add nodejs apt repo
    curl -sL https://deb.nodesource.com/setup_16.x | sudo -E bash -

    # add yarn apt repo
    curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | sudo apt-key add -

    # add yarn apt repo
    echo "deb https://dl.yarnpkg.com/debian/ stable main" | sudo tee /etc/apt/sources.list.d/yarn.list

    # add nodejs and yarn libraries
    while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
nodejs
yarn
EOF
)
fi

if [ $postgres_setup = "y" ]; then
    # add postgres apt repo
    sudo sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt $(lsb_release -cs)-pgdg main" > /etc/apt/sources.list.d/pgdg.list'
    wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -

    # add postgresql libraries
    while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
postgresql
postgresql-15-postgis-3
libpq-dev
libgmp3-dev
EOF
)
fi

if [ $mosquitto_setup = "y" ]; then
    # add mosquitto libraries
    while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
mosquitto
mosquitto-clients
EOF
)
fi

if [ $redis_setup = "y" ]; then
    # add redis libraries
    while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
redis-server
EOF
)
fi

if [ $grafana_setup = "y" ]; then
    # add grafana apt repo
    wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -
    echo "deb https://packages.grafana.com/oss/deb stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list

    # add grafana libraries
    while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
grafana
EOF
)
fi

if [ $nginx_setup = "y" ]; then
    # add nginx libraries
    while read -r p ; do libraries="$libraries $p" ; done < <(cat << "EOF"
nginx
certbot
python3-certbot-nginx
EOF
)
fi

# install libraries we are using
sudo apt-get update

echo "Installing libraries $libraries..."

sudo apt-get install $libraries -y

echo "Finished setting up custom APT repositories."
read -p "Press enter to continue."

# -------------------- FRONTEND SETUP --------------------
if [ $frontend_setup = "y" ]; then
    echo "Setting up frontend..."
    chmod +x ./scripts/frontend.sh
    ./scripts/frontend.sh
    read -p "Frontend set up. Press enter to continue."
else
    echo "Skipping frontend setup..."
fi

# -------------------- POSTGRES SETUP --------------------
if [ $postgres_setup = "y" ]; then
    echo "Setting up postgres..."
    chmod +x ./scripts/postgres.sh
    ./scripts/postgres.sh
    read -p "Postgres set up. Press enter to continue."
else
    echo "Skipping postgres setup..."
fi

# -------------------- MOSQUITTO SETUP --------------------
if [ $mosquitto_setup = "y" ]; then
    echo "Setting up mosquitto..."
    chmod +x ./scripts/mosquitto.sh
    ./scripts/mosquitto.sh
    read -p "Mosquitto set up. Press enter to continue."
else
    echo "Skipping mosquitto setup..."
fi

# -------------------- REDIS SETUP --------------------
if [ $redis_setup = "y" ]; then
    echo "Setting up redis..."
    chmod +x ./scripts/redis.sh
    ./scripts/redis.sh
    read -p "Redis set up. Press enter to continue."
else
    echo "Skipping redis setup..."
fi

# -------------------- BACKEND SETUP --------------------
if [ $backend_setup = "y" ]; then
    echo "Setting up backend..."
    chmod +x ./scripts/backend.sh
    ./scripts/backend.sh
    read -p "Backend set up. Press enter to continue."
else
    echo "Skipping backend setup..."
fi

# -------------------- GRAFANA SETUP --------------------
if [ $grafana_setup = "y" ]; then
    echo "Setting up grafana..."
    chmod +x ./scripts/grafana.sh
    ./scripts/grafana.sh
    read -p "Grafana set up. Press enter to continue."
else
    echo "Skipping grafana setup..."
fi

# -------------------- OPEN PORTS --------------------
echo "Opening ports for nginx and mqtt..."
echo "Note: we do not open postgres ports due to security. If you need to access the db directly, you can use this server as a jump host. (Or you can use pgadmin.)"

echo "This script assumes you are using iptables and not ufw to manage your server's firewall. This is the default on Oracle's Ubuntu images."
# open ports for nginx
sudo iptables -I INPUT 6 -m state --state NEW -p tcp --dport 80 -j ACCEPT
sudo iptables -I INPUT 6 -m state --state NEW -p tcp --dport 443 -j ACCEPT
# open ports for mqtt
sudo iptables -I INPUT 6 -m state --state NEW -p tcp --dport 1883 -j ACCEPT
sudo netfilter-persistent save

echo "Finished opening ports for nginx and mqtt."
read -p "Press enter to continue."

# -------------------- NGINX SETUP --------------------
if [ $nginx_setup = "y" ]; then
    echo "Setting up nginx..."
    chmod +x ./scripts/nginx.sh
    ./scripts/nginx.sh
    read -p "Nginx set up. Press enter to continue."
else
    echo "Skipping nginx setup..."
fi