cf_good_install_msg="\n\nCloudflare installed and ready to start a tunnel! Please visit cloudflare tunnels and create a new tunnel for this device and paste the command here. It should look like: \nsudo cloudflared service install <long_id>\n\n"
cf_bad_install_msg="Please install cloudflare manually, as the install script does not support your OS. https://github.com/cloudflare/cloudflared/releases/latest"
cf_installed=false

# TODO: maybe add a check to see if the user is running this as root or not

echo "Beginning setup of ground station...\n"

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


echo "\nGround station package setup complete!\n"


# Ask if the user wants to install cloudflare
read -p "Would you like to install cloudflare? (y/n): " cf_install

echo "\n"

if [ $cf_install = "y" ]; then
    # verify that this is linux
    if [ "$(uname)" = "Linux" ]; then
        # verify running arm64 or aarch64
        if [ "$(uname -m)" = "aarch64" ] || [ "$(uname -m)" = "arm64" ]; then
            # install cloudflare
            curl -L --output cloudflared.deb https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-arm64.deb
            sudo dpkg -i cloudflared.deb

            cf_installed=true
        fi
    fi
fi

# install the necessary python packages
pip3 install -r requirements.txt

# TODO: setup systemd service

echo "\nGround station setup complete!\n"

# Check if cloudflare is installed and print message
if [ $cf_installed = true ]; then
    echo $cf_good_install_msg
else
    if [ $cf_install = "y" ]; then
        echo $cf_bad_install_msg
    fi
fi

# TODO: echo info about systemd service

echo "\nPlease edit the config.yaml file to your liking, and then reboot your device to complete the setup process."
