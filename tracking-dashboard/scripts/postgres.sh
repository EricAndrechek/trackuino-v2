# start postgres
sudo systemctl daemon-reload
sudo systemctl enable postgresql
sudo systemctl start postgresql

read -p "Would you like to specify your own postgres passwords (y), or use randomly generated ones (n)? (y/n): " postgres_passwords

postgres_admin_password=""
postgres_grafana_password=""
postgres_server_password=""

if [ $postgres_passwords = "y" ]; then
    read -p "What would you like to set the postgres admin password to?: " postgres_admin_password
    read -p "What would you like to set the postgres grafana password to?: " postgres_grafana_password
    read -p "What would you like to set the postgres server password to? (this is the one you will later add to the config): " postgres_server_password
else
    postgres_admin_password=$(openssl rand -base64 32)
    postgres_grafana_password=$(openssl rand -base64 32)
    postgres_server_password=$(openssl rand -base64 32)
fi

# setup postgres
cd ~postgres/ || cd /var/lib/postgresql
sudo -u postgres psql -c "ALTER USER postgres PASSWORD '$postgres_admin_password';"

# create grafana user
sudo -u postgres psql -c "CREATE USER grafana WITH PASSWORD '$postgres_grafana_password'; GRANT USAGE ON SCHEMA schema TO grafanareader; GRANT SELECT ON schema.table TO grafanareader;"

# create server user
sudo -u postgres psql -c "CREATE USER server WITH PASSWORD '$postgres_server_password'; ALTER USER server WITH CREATEDB;"
sudo -u postgres psql -c "CREATE DATABASE db OWNER server; GRANT ALL PRIVILEGES ON DATABASE db TO server;"

# create postgis extension for db
sudo -u postgres psql -d db -c "CREATE EXTENSION postgis;"

echo "Postgres setup complete!"

# let user know passwords
echo "Your postgres 'admin' password is: $postgres_admin_password"
echo "Your postgres 'grafana' password is: $postgres_grafana_password"
echo "Your postgres 'server' password is: $postgres_server_password"

# save passwords to file
sudo tee $DIR/.postgres_passwords.txt > /dev/null <<EOT
postgres_admin_password=$postgres_admin_password
postgres_grafana_password=$postgres_grafana_password
postgres_server_password=$postgres_server_password
EOT

read -p "Press enter once you have saved these passwords somewhere safe. They have also been saved to $DIR/.postgres_passwords.txt"

# TODO: setup pgadmin4?