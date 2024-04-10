# ask for email for certbot
read -p "What is your email address? (for certbot): " email

# setup nginx
# ask for root domain
read -p "What is your root domain where your frontend will be? (e.g. example.com): " root_domain

if [ $backend_setup = "y" ]; then
    # ask for api domain
    read -p "What is your api domain? (e.g. api.example.com): " api_domain
    # build nginx config for backend
    sudo tee /etc/nginx/sites-available/$api_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $api_domain;
    location / {
        proxy_pass http://localhost:5000;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
    sudo ln -s /etc/nginx/sites-available/$api_domain /etc/nginx/sites-enabled/
    sudo certbot run -n --nginx --agree-tos -d $api_domain --email $email --redirect
fi

# set default site to redirect to root domain
sudo rm /etc/nginx/sites-enabled/default
sudo tee /etc/nginx/sites-available/default > /dev/null <<EOT
server {
    listen 80 default_server;
    listen [::]:80 default_server;
    server_name _;
    return 301 https://$root_domain\$request_uri;
}
EOT

sudo ln -s /etc/nginx/sites-available/default /etc/nginx/sites-enabled/

# setup wss
sudo certbot certonly --manual --preferred-challenges dns -d mqtt.$root_domain --email $email

# copy certs to mosquitto
sudo mkdir -p /etc/mosquitto/certs

sudo cp /etc/letsencrypt/live/mqtt.$root_domain/fullchain.pem /etc/mosquitto/certs/fullchain.pem
sudo cp /etc/letsencrypt/live/mqtt.$root_domain/privkey.pem /etc/mosquitto/certs/privkey.pem

# get cert at /etc/letsencrypt/live/mqtt.$root_domain/fullchain.pem
# save the first cert, starting at the first index of "-----BEGIN CERTIFICATE----" end ending after the first index of "-----END CERTIFICATE-----" to /etc/letsencrypt/live/mqtt.$root_domain/cert.pem

cert=$(sudo cat /etc/letsencrypt/live/mqtt.$root_domain/fullchain.pem)
cert_start
cert_end
cert_start=$(echo $cert | grep -b -o "-----BEGIN CERTIFICATE-----" | head -n 1 | cut -d: -f1)
cert_end=$(echo $cert | grep -b -o "-----END CERTIFICATE-----" | head -n 1 | cut -d: -f1)
cert=$(echo $cert | cut -c $cert_start-$cert_end)
sudo tee /etc/mosquitto/certs/cert.pem > /dev/null <<EOT
$cert
EOT

# set permissions
sudo chown mosquitto:mosquitto /etc/mosquitto/certs/fullchain.pem /etc/mosquitto/certs/privkey.pem /etc/mosquitto/certs/cert.pem
sudo chmod 400 /etc/mosquitto/certs/fullchain.pem /etc/mosquitto/certs/privkey.pem /etc/mosquitto/certs/cert.pem
sudo chmod +x /etc/mosquitto
sudo chmod +x /etc/mosquitto/certs


# edit /etc/mosquitto/mosquitto.conf to include the fullchain, cert, and key
sudo tee /etc/mosquitto/mosquitto.conf > /dev/null <<EOT
listener 1883
allow_anonymous true

listener 1884
protocol websockets
cafile /etc/mosquitto/certs/fullchain.pem
certfile /etc/mosquitto/certs/cert.pem
keyfile /etc/mosquitto/certs/privkey.pem
EOT


# restart nginx
sudo systemctl restart nginx
