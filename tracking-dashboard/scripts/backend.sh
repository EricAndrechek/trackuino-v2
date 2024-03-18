cd $DIR/backend
# create virtual environment
python3 -m venv venv
# activate virtual environment
source venv/bin/activate
# upgrade pip
pip install --upgrade pip
# install requirements
pip install -r requirements.txt
# deactivate virtual environment
deactivate

# setup gunicorn systemd service
# TODO: tweak gunicorn settings
sudo tee /etc/systemd/system/api.service > /dev/null <<EOT
[Unit]
Description=api
After=network.target

[Service]
User=ubuntu
WorkingDirectory=$DIR/backend
ExecStart=$DIR/backend/venv/bin/gunicorn -c gunicorn.conf.py
Restart=always

[Install]
WantedBy=multi-user.target
EOT

# setup rqworker target
sudo tee /etc/systemd/system/api-tasks.target > /dev/null <<EOT
[Unit]
Description=API RQ worker pool

[Install]
WantedBy=multi-user.target
EOT

# setup rqworker systemd service
sudo tee /etc/systemd/system/api-tasks@.service > /dev/null <<EOT
[Unit]
Description=API task worker %I
After=network.target

[Service]
User=ubuntu
WorkingDirectory=$DIR/backend
ExecStart=$DIR/backend/venv/bin/rq worker api-tasks
Restart=always

[Install]
WantedBy=api-tasks.target
EOT

# enable api service
# TODO: tweak number of workers
# https://docs.gunicorn.org/en/stable/design.html#how-many-workers
sudo systemctl daemon-reload
sudo systemctl enable api
sudo systemctl enable api-tasks@{1..4}
sudo systemctl start api
sudo systemctl start api-tasks@{1..4}


# all info from: https://blog.miguelgrinberg.com/post/running-a-flask-application-as-a-service-with-systemd
