# before running this, set the time with:
# sudo date -s "2021-09-01 18:00:00"

# clone this repo and run this script


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

# Install the required packages
sudo apt install gpsd libgps-dev gpsd-clients chrony minicom -y

sudo apt-get install libusb-1.0-0-dev git cmake pkg-config build-essential
git clone https://github.com/rtlsdrblog/rtl-sdr-blog
cd rtl-sdr-blog/
mkdir build
cd build
cmake ../ -DINSTALL_UDEV_RULES=ON
make
sudo make install
sudo cp ../rtl-sdr.rules /etc/udev/rules.d/
sudo ldconfig

cd ../../

# check gpsd and chrony status
sudo systemctl status gpsd
sudo systemctl status chrony

# enable gpsd and chrony
sudo systemctl enable gpsd
sudo systemctl enable chrony

echo 'blacklist dvb_usb_rtl28xxu' | sudo tee --append /etc/modprobe.d/blacklist-dvb_usb_rtl28xxu.conf

sudo apt install direwolf -y

git clone https://github.com/wottreng/SIM7080G-NB-IoT-Cat-M-LTE-GPS.git
cd SIM7080G-NB-IoT-Cat-M-LTE-GPS
cd refs
sudo chmod +x pin
