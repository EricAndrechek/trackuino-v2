# ask for email for certbot
read -p "What is your email address? (for certbot): " email

# setup nginx
# ask for root domain
read -p "What is your root domain where your frontend will be? (e.g. example.com): " root_domain

if [ $frontend_setup = "y" ]; then
    # build nginx config for frontend
    # TODO: setup file location, file extension stripping, and file serving
    sudo tee /etc/nginx/sites-available/$root_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $root_domain;
    root /var/www/html;
    index index.html;
    location / {
        try_files \$uri \$uri/ =404;
    }
}
EOT
    sudo ln -s /etc/nginx/sites-available/$root_domain /etc/nginx/sites-enabled/
    sudo certbot run -n --nginx --agree-tos -d $root_domain --email $email --redirect
fi

if [ $postgres_setup = "y" ]; then
    # ask for pgadmin domain
    read -p "What is your pgadmin domain? (e.g. pgadmin.example.com): " postgres_domain
    # build nginx config for pgadmin
    sudo tee /etc/nginx/sites-available/$postgres_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $postgres_domain;
    location / {
        proxy_pass http://localhost:5050;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
    sudo ln -s /etc/nginx/sites-available/$postgres_domain /etc/nginx/sites-enabled/
    sudo certbot run -n --nginx --agree-tos -d $postgres_domain --email $email --redirect
fi

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

if [ $mqtt_setup = "y" ]; then
    # ask for mqtt domain
    read -p "What is your mqtt domain? (e.g. mqtt.example.com): " mqtt_domain
    # build nginx config for mqtt
    # TODO: support wss and 1883 port for mqtt?
    sudo tee /etc/nginx/sites-available/$mqtt_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $mqtt_domain;
    location / {
        proxy_pass http://localhost:9001;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
    sudo ln -s /etc/nginx/sites-available/$mqtt_domain /etc/nginx/sites-enabled/
    sudo certbot run -n --nginx --agree-tos -d $mqtt_domain --email $email --redirect
fi

if [ $grafana_setup = "y" ]; then
    # ask for grafana domain
    read -p "What is your grafana domain? (e.g. status.example.com): " grafana_domain
    # build nginx config for grafana
    sudo tee /etc/nginx/sites-available/$grafana_domain > /dev/null <<EOT
server {
    listen 80;
    listen [::]:80;
    server_name $grafana_domain;
    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host \$host;
        proxy_set_header X-Forwarded-Proto \$scheme;
        proxy_set_header X-Forwarded-For \$remote_addr;
    }
}
EOT
    sudo ln -s /etc/nginx/sites-available/$grafana_domain /etc/nginx/sites-enabled/
    sudo certbot run -n --nginx --agree-tos -d $grafana_domain --email $email --redirect
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

# restart nginx
sudo systemctl restart nginx