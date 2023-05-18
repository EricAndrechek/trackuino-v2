cf_installed=false
good_install=false

# TODO: maybe add a check to see if the user is running this as root or not

echo "Beginning setup of ground station...\n"

if [ "$(uname)" = "Linux" ]; then
    # verify running arm64 or aarch64
    if [ "$(uname -m)" = "aarch64" ] || [ "$(uname -m)" = "arm64" ]; then

        # Update and upgrade
        sudo apt update
        sudo apt upgrade -y
        sudo apt autoremove -y

        # Install the required packages
        sudo apt install rtl-sdr multimon-ng sox -y

        # Setup udev rules
        wget https://github.com/osmocom/rtl-sdr/raw/master/rtl-sdr.rules
        sudo mv rtl-sdr.rules /etc/udev/rules.d/

        # Reload udev rules
        sudo udevadm control --reload-rules
        sudo udevadm trigger

        echo "\nGround station package setup complete\!\n"

        # Ask if the user wants to install cloudflare
        read -p "Would you like to install cloudflare? (y/n): " cf_install

        echo "\n"

        if [ $cf_install = "y" ]; then
            # install cloudflare
            curl -L --output cloudflared.deb https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-arm64.deb
            sudo dpkg -i cloudflared.deb

            cf_installed=true
        fi

        # change hostname at /boot/efi/user-data
        read -p "Would you like to change the hostname? (y/n): " hostname_change

        if [ $hostname_change = "y" ]; then
            og_hostname=$(hostnamectl --static)
            read -p "What would you like the hostname to be? (no spaces): " hostname
            sudo sed -i "s/$og_hostname/$hostname/g" /boot/efi/user-data
        fi

        echo "To complete the hostname change, please reboot your device.\n"

        echo "To setup custom DNS, please edit the /boot/efi/network-config file.\n"

        # setup netdata
        read -p "Would you like to install netdata to monitor your ground station's system metrics (CPU, ram, temp, etc) (Note: this assumes you are running a Netdata parent client elsewhere. If you don't want to do this, setup Netdata yourself - it's super easy)? (y/n): " netdata_install
        if [ $netdata_install = "y" ]; then
            # install netdata
            wget -O /tmp/netdata-kickstart.sh https://my-netdata.io/kickstart.sh && sh /tmp/netdata-kickstart.sh --no-updates --stable-channel --disable-telemetry
            
            # copy stream config and modify to send data to parent
            sudo cp /etc/netdata/stream.conf /lib/netdata/conf.d/stream.conf
            
            sudo sed -zi "s/# Enable this on child nodes, to have them send metrics.\n    enabled = no/# Enable this on child nodes, to have them send metrics.\n    enabled = yes/g" /etc/netdata/stream.conf

            read -p "What is the hostname of your Netdata parent client? (Note: this is the hostname of the parent client): " netdata_parent_hostname
            sudo sed -i "s/destination =/destination = $netdata_parent_hostname/g" /etc/netdata/stream.conf
            
            read -p "What is your Netdata API key? (Note: this is the API key of the parent client): " netdata_api_key
            sudo sed -i "s/api key =/api key = $netdata_api_key/g" /etc/netdata/stream.conf

            # copy preconfigured netdata config
            sudo cp netdata.conf /etc/netdata/netdata.conf

            echo "Restarting Netdata...\n"
            sudo systemctl restart netdata

            echo "Netdata setup complete\!"
            echo "To view your Netdata dashboard, go to http://$netdata_parent_hostname:19999"
            echo "If no data is showing, check\n\nsudo tail -f /var/log/netdata/error.log\n\nfor errors."
        fi

        # TODO: setup systemd service
        

        good_install=true
    fi
fi

if [ $good_install = false ]; then
    echo "Dependency installation only works on arm64/aarch64 devices running Linux. Please consult the docs for more information.\n"
fi

# create python virtual environment
python3 -m venv venv

# activate the virtual environment
source venv/bin/activate

# upgrade pip
pip3 install --upgrade pip

# install the necessary python packages
pip3 install -r requirements.txt

echo "\nGround station setup complete\!\n"

# TODO: echo info about systemd service

echo "\nPlease edit the config.yaml file to your liking, and then reboot your device to complete the setup process."
