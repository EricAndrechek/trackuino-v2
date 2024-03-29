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

        # enable serial port hardware, remote GPIO, I2C, SPI, 1-Wire, and SSH
        sudo raspi-config nonint do_serial 0
        sudo raspi-config nonint do_i2c 0
        sudo raspi-config nonint do_spi 0
        sudo raspi-config nonint do_onewire 0
        sudo raspi-config nonint do_ssh 0

        sudo apt-get install minicom -y

        sudo apt-get install cmake -y
        sudo apt-get install libasound2-dev -y
        sudo apt-get install libudev-dev -y

        # Install the required packages
        sudo apt install rtl-sdr multimon-ng sox gpsd libgps-dev gpsd-clients -y

        # Setup udev rules
        wget https://github.com/osmocom/rtl-sdr/raw/master/rtl-sdr.rules
        sudo mv rtl-sdr.rules /etc/udev/rules.d/

        # Reload udev rules
        sudo udevadm control --reload-rules
        sudo udevadm trigger

        sudo apt install direwolf -y

        cp /usr/local/share/doc/direwolf/examples/direwolf.conf ~/direwolf.conf

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

sudo pip3 install paho-mqtt pyserial

# install the necessary python packages
pip3 install -r requirements.txt

echo "\nGround station setup complete\!\n"

# TODO: echo info about systemd service

echo "\nPlease edit the config.yaml file to your liking, and then reboot your device to complete the setup process."
