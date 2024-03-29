# Ground Station <!-- omit in toc -->
<!-- Badges will go here? -->

A simple and hackable python and linux based APRS IGate for receiving your balloons' tracking data via HAM.

For the balloon tracker, check the [balloon-tracker](../balloon-tracker/) page.

For tracking chase vehicles that aren't running this receiver or don't need to be, check [vehicle-tracker](../vehicle-tracker/).

For tracking the balloon once it has landed and triggering the cut-down burn wire, setup a [handheld-locator](../handheld-locator/)

## Table of Contents <!-- omit in toc -->

- [1. Hardware](#1-hardware)
  - [1.1. Computing Device](#11-computing-device)
  - [1.2. Software-Define Radio (SDR)](#12-software-define-radio-sdr)
  - [1.3. Internet Uplink](#13-internet-uplink)
    - [1.3.1. SIM Card Hat](#131-sim-card-hat)
  - [1.4. Display](#14-display)
  - [1.5. GPS](#15-gps)
- [2. Setup](#2-setup)
  - [2.1. Install Ubuntu Server](#21-install-ubuntu-server)
  - [2.2. Download \& Install Dependencies](#22-download--install-dependencies)
    - [2.2.1. Automatic Setup](#221-automatic-setup)
    - [2.2.2. Manual Setup](#222-manual-setup)
  - [2.3. (Optional) Setup Cloudflare Tunnel](#23-optional-setup-cloudflare-tunnel)
    - [2.3.1. After installing `cloudflared`](#231-after-installing-cloudflared)
- [3. Additional Walk-Throughs](#3-additional-walk-throughs)
  - [3.1. Config.yml Settings](#31-configyml-settings)
  - [3.2. Web-Interface Setup \& Forwarding](#32-web-interface-setup--forwarding)
- [4. Appendix](#4-appendix)
  - [4.1. Direwolf Debugging](#41-direwolf-debugging)
  - [4.2. Libraries](#42-libraries)
  - [4.3. References](#43-references)
  - [4.4. SDR Software](#44-sdr-software)
  - [4.5. Related Works](#45-related-works)

# 1. Hardware

This is designed to be run with ARM-based linux machines and software-defined radios, although it can be hacked to work on other OSes and with hardware TNCs. Check [EricAndrechek/aprs-receiver](https://github.com/EricAndrechek/aprs-receiver) for a similar project but for the Kenwood TNC.

## 1.1. Computing Device

Normally a Raspberry Pi would be the suggested hardware for this, although they have gotten rather expensive lately, so something like the [Libre Computer Board](https://libre.computer/products/aml-s905x-cc/) could be used instead. This is what it was tested and built on.

## 1.2. Software-Define Radio (SDR)

Check out [this](https://www.rtl-sdr.com/buy-rtl-sdr-dvb-t-dongles/) buying guide to determine exactly what you are looking for. For most APRS purposes (in the US), you want things that can receive at the 144.390MHz range. I have found much success with receivers with an RTL2832U chip.

Additionally you will need a good antenna for receiving from longer distances. Do note, that the antenna should not be any further from the USB than absolutely necessary, so if you need the USB receiver/antenna to be further from the computer, add a longer USB cable (or active USB cable if it is above 5m).

## 1.3. Internet Uplink

There are several different methods one could use to upload APRS and GPS data to the internet. This tools assumes all of that setup is done by the user and does not handle any of it. Everything should work fine so long as your device has an internet connection and has port 14580 open in the firewall.

### 1.3.1. SIM Card Hat

AT COMMANDS:

AT+CSTT="hologram"

// Facility Lock
AT+CLCK
Set to LTE mode
AT+CNMP=38
AT+COPS Operator Selection
AT+CREG Network Registration

## 1.4. Display

This program has an optional ability to start a web-server to show live stats about any call-signs you want it to track. This web interface can be viewed on other devices on the same network, can be shown on a display for the computer and optionally automatically launched in kiosk mode on-boot, or can be accessed over the internet by using a service like Cloudflare Tunnels.

While buying a display is not strictly necessary, it can make the initial setup process easier until SSH and tunnelling are all working, and can make debugging and reading in the field easier where cellular availability may not always exist.

## 1.5. GPS

This software can optionally integrate the same functionality as the [vehicle-tracker](../vehicle-tracker/). In order to track your receiver vehicle/device without separate vehicle-tracker hardware, you will need a USB-enabled GPS device. Ideally find one that says it supports linux so that you don't run into driver issues.

# 2. Setup

## 2.1. Install Ubuntu Server

Start by installing Ubuntu Server 22.04 (or some other distribution or OS, but Ubuntu Server 22.04 is lightweight and tested)

Download an Ubuntu Server iso from the internet. [Here's the download for the Libre Computer](https://distro.libre.computer/ci/ubuntu/22.04/)

Put the iso into a tool like [Raspberry Pi Imager](https://www.raspberrypi.com/software/) and install it onto a microSD card (for installing on SBCs) or a USB (for desktop/laptop style computers). Note that for single board computers, the faster the microSD the better.

## 2.2. Download & Install Dependencies

1. Connect your device to the internet! For fellow people at the University of Michigan, you can connect following [this article](https://teamdynamix.umich.edu/TDClient/76/Portal/KB/ArticleDet?ID=5268) to register at [netreg.engin.umich.edu](https://netreg.engin.umich.edu/). If using ethernet you may need to contact ITS.

2. Install the necessary libraries and begin the setup process. You can either clone this repository and run the setup script, or follow the manual steps below.

### 2.2.1. Automatic Setup

```bash
git clone --no-checkout --depth=1 --filter=tree:0 https://github.com/EricAndrechek/trackuino-v2.git
cd trackuino-v2
git sparse-checkout set --no-cone ground-station
git checkout
cd ground-station
chmod +x setup.sh
./setup.sh
```

### 2.2.2. Manual Setup

1. Update the OS and kernel to the newest version. This may take a while.

```bash
sudo apt update
sudo apt upgrade -y
sudo apt autoremove -y
```

2. Install the necessary tools and libraries.

```bash
sudo apt install rtl-sdr multimon-ng sox -y
```

3. Setup rtl udev rules to give proper access to the USB device.

```bash
wget https://github.com/osmocom/rtl-sdr/raw/master/rtl-sdr.rules
sudo mv rtl-sdr.rules /etc/udev/rules.d/
```

4. Restart udev for the rule policy changes to take effect

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

5. Now, we need to clone this repository (specifically the ground station functionality), and begin the setup.

```bash
git clone --no-checkout --depth=1 --filter=tree:0 https://github.com/EricAndrechek/trackuino-v2.git
cd trackuino-v2
git sparse-checkout set --no-cone ground-station
git checkout
cd ground-station
```

Your file structure should now look something like `~/trackuino-v2/ground-station/`.

6. Install the python dependencies

```bash
pip3 install -r requirements.txt
```

7. Update the hostname and DNS by modifying `/boot/efi/user-data` and `/boot/efi/network-config`.

Hostname:

```bash
sudo nano /boot/efi/user-data
```

And it should include this line:

```yaml
hostname: gs-1
```

DNS Servers:

```bash
sudo nano /boot/efi/network-config
```

Which should look something like:

```yaml
version: 2
ethernets:
  eth0:
    dhcp4: true
    optional: true
    nameservers: 
      addresses: [1.1.1.1, 8.8.8.8]
```

8. Edit the config.yml file to suit your needs. See [this section](#31-configyml-settings) for more information.

9. Reboot! (`sudo reboot`)

## 2.3. (Optional) Setup Cloudflare Tunnel

*Note*: While not strictly necessary, this is **strongly** recommended as it enables troubleshooting and fixing devices while in the field remotely. It also makes managing a larger fleet of devices significantly easier.

For the purposes of this tutorial, we will be using Cloudflare, although none of the functionality baked into the code is Cloudflare-specific, so building your own tunnelling system with NGinx on a self-hosted server, or using a service like Tailscale is also an option.

Instead of following my steps, feel free to follow along with [Cloudflare's official guide](https://developers.cloudflare.com/cloudflare-one/connections/connect-apps/use-cases/ssh/#connect-to-ssh-server-with-cloudflared-access)

The setup script will automatically prompt you and offer to install cloudflared for you, but if you want to do it manually, follow the instructions below.


<details>
<summary>
  Manual Cloudflare Tunnel Setup
</summary>
  Create a new cloudflare tunnel and copy the install and run connectors commands

  ```bash
  curl -L --output cloudflared.deb https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-arm64.deb

  sudo dpkg -i cloudflared.deb
  ```
</details>
<br>

### 2.3.1. After installing `cloudflared`

1. Create a new cloudflare tunnel and copy the install and run connectors commands, which should look something like this:

```bash
sudo cloudflared service install <long_id>
```

2. Create a cloudflare zero-trust access policy for the tunnel

3. Create a cloudflare zero-trust application for the tunnel (select self-hosted) and add the policy to it.

4. (SSH only, not Web Interface) Go to the application's settings and turn on browser-rendering.

5. Visit the application's URL in a browser and login with your access policy. 

6. (Optional) Create a new public hostname for the same tunnel for the web interface and repeat steps 2-5.

You should now be able to access your device via SSH with the cloudeflared proxy or SSH in the browser. If you are using the web interface, you should also be able to access it via the browser.

# 3. Additional Walk-Throughs

## 3.1. Config.yml Settings

Edit the config.yml file to suit your needs. The below (hidden) section documents in more detail what each section of the config outlines should the config file's comments not be sufficient.

<details>
    <summary>
        config.yml settings
    </summary>
    TODO: config stuff here
</details>
<br>

## 3.2. Web-Interface Setup & Forwarding

# 4. Appendix

## 4.1. Direwolf Debugging

When multiple modems are configured per channel, a simple spectrum display reveals which decoders
picked up the signal properly.
| means a frame was received with no error.
: means a frame was received with a single bit error. (FIX_BITS 1 or higher configured.)
. means a frame was received with multiple errors. (FIX_BITS 2 or higher configured.)
_ means nothing was received on this decoder.

Here are some samples and what they mean.
___|___ Only the center decoder (e.g. 1600/1800 Hz) was successful.
_|||___ 3 different lower frequency modems received it properly.
Assuming USB operation, the transmitting station is probably a
little low in frequency.
___|||: 3 different higher frequency modems received it with no error.
The highest one received it with a single bit error.

Check out the resources below for additional information and readings:

## 4.2. Libraries

- [librtlsdr](https://github.com/steve-m/librtlsdr)
- [multimon-ng](https://github.com/EliasOenal/multimon-ng)
- [SoX](https://github.com/chirlu/sox)
- [aprslib](https://github.com/rossengeorgiev/aprs-python)
- [aprs-symbols](https://github.com/hessu/aprs-symbols)
- [aprs-symbol-index](https://github.com/hessu/aprs-symbol-index)
- [aprs-deviceid](https://github.com/aprsorg/aprs-deviceid)

## 4.3. References

- [RTL-SDR Blog](https://www.rtl-sdr.com/about-rtl-sdr/)
- [osmosom.org rtl-sdr](https://osmocom.org/projects/rtl-sdr/wiki/Rtl-sdr)
- [Rtl_fm Guide](http://kmkeen.com/rtl-demod-guide/index.html)
- [Radio Reference](https://www.radioreference.com/db/) (useful for seeing nearby frequency allocations)

## 4.4. SDR Software

Ultimately, just follow this website list and use what works best for you: [rtl-sdr.com/big-list-rtl-sdr-supported-software/](https://www.rtl-sdr.com/big-list-rtl-sdr-supported-software/)

The applications I found the easiest and most useful were:

- [SDR#](https://airspy.com/download/) (Windows)
- [GQRX](http://gqrx.dk/) (Linux/Mac/WSL)
- [SDR++](https://github.com/AlexandreRouma/SDRPlusPlus) (Linux/Mac/Windows)

## 4.5. Related Works

- [Raspberry Pi + RTL SDR dongle = APRS Rx only gate](https://e.pavlin.si/2014/12/10/raspberry-pi-rtl-sdr-dongle-aprs-rx-only-gate-part-two/) (this exact project minus the packaging, easy setup/config, and web interface)
- [APRS CLI Decoding](https://gist.github.com/jj1bdx/8ab103e774c81d2c068d455ab862b72e) (effectively what this project is doing without a few extra features unqiue to this project)
- [APRS iGate RX from DVB-T tuner](http://sq7mru.blogspot.com/2013/08/aprs-igate-rx-z-tunera-dvb-t.html)
- [pymultimonaprs](https://github.com/asdil12/pymultimonaprs) (very similar, but was python 2, outdated, and didn't have the features we wanted)
- [pymma](https://github.com/ampledata/pymma) (fork of pymultimonapr)
- [aprs-receiver](https://github.com/EricAndrechek/aprs-receiver) (a previous iteration of this project, but for hardware TNCs and built in a rush :sweat_smile:)
- [pypacket](https://github.com/cceremuga/pypacket) (now defunct, but was a similar project to this one)
