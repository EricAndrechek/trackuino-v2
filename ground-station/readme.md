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
  - [1.4. Display](#14-display)
  - [1.5. GPS](#15-gps)
- [2. Setup](#2-setup)
  - [2.1. Install Ubuntu Server](#21-install-ubuntu-server)
  - [2.2. Setup Linux](#22-setup-linux)
  - [2.3. Setup Tunnels](#23-setup-tunnels)
  - [2.4. Setup the Ground Station](#24-setup-the-ground-station)
- [3. Additional Walk-Throughs](#3-additional-walk-throughs)
  - [3.1. Config.yml Settings](#31-configyml-settings)
  - [3.2. Web-Interface Setup \& Forwarding](#32-web-interface-setup--forwarding)
- [4. Appendix](#4-appendix)
  - [Libraries](#libraries)
  - [References](#references)
  - [SDR Software](#sdr-software)
  - [Related Works](#related-works)

# 1. Hardware

This is designed to be run with ARM-based linux machines and software-defined radios, although it can be hacked to work on other OSes and with hardware TNCs. Check [EricAndrechek/aprs-receiver](https://github.com/EricAndrechek/aprs-receiver) for a similar project but for the Kenwood TNC.

## 1.1. Computing Device

Normally a Raspberry Pi would be the suggested hardware for this, although they have gotten rather expensive lately, so something like the [Libre Computer Board](https://libre.computer/products/aml-s905x-cc/) could be used instead. This is what it was tested and built on.

## 1.2. Software-Define Radio (SDR)

Check out [this](https://www.rtl-sdr.com/buy-rtl-sdr-dvb-t-dongles/) buying guide to determine exactly what you are looking for. For most APRS purposes (in the US), you want things that can receive at the 144.390MHz range. I have found much success with receivers with an RTL2832U chip.

Additionally you will need a good antenna for receiving from longer distances. Do note, that the antenna should not be any further from the USB than absolutely necessary, so if you need the USB receiver/antenna to be further from the computer, add a longer USB cable (or active USB cable if it is above 5m).

## 1.3. Internet Uplink

There are several different methods one could use to upload APRS and GPS data to the internet. This tools assumes all of that setup is done by the user and does not handle any of it. Everything should work fine so long as your device has an internet connection and has port 14580 open in the firewall.

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

## 2.2. Setup Linux

1. Connect your device to the internet!

2. Update the OS and kernel to the newest version. This may take a while.

```bash
sudo apt update
sudo apt upgrade -y
```

3. Install the necessary tools and libraries.

```bash
sudo apt install openssh-server git nano curl wget rtl-sdr multimon-ng sox -y
```

1. Setup rtl udev rules to give proper access to the USB device.

```bash
wget https://github.com/osmocom/rtl-sdr/raw/master/rtl-sdr.rules
sudo mv rtl-sdr.rules /etc/udev/rules.d/
```

5. Restart udev for the rule policy changes to take effect

```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## 2.3. Setup Tunnels

*Note*: While not strictly necessary, this is **strongly** recommended as it enables troubleshooting and fixing devices while in the field remotely. It also makes managing a larger fleet of devices significantly easier.

For the purposes of this tutorial, we will be using Cloudflare, although none of the functionality baked into the code is Cloudflare-specific, so building your own tunnelling system with NGinx on a self-hosted server, or using a service like Tailscale is also an option.

1. Create a new cloudflare tunnel and copy the install and run connectors commands

```bash
curl -L --output cloudflared.deb https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-arm64.deb && 

sudo dpkg -i cloudflared.deb && 

sudo cloudflared service install <super long string of characters>
```

<!-- Haven't tested this part yet:

2. Login to cloudflare

```bash
cloudflared tunnel login
```

3. Create tunnel (replace GS-1 with your tunnel name)

```bash
cloudflare tunnel create GS-1
```

4. Go to cloudflare and create a self-hosted zero-trust application.

5. Edit the config file

```bash
cd .cloudflared
nano config.yml
```

and add the following

```yml
tunnel: <tunnel-uuid>
credentials-file: /root/.cloudflared/<tunnel-uuid>.json
warp-routing:
    enabled: true
ingress:
  - hostname: <application-url>
    service: ssh://localhost:22
  - service: http_status:404
```

6. Next, set the dns tunnel routes, and enable the tunnel config as a systemd service so it runs automatically.

```bash
cloudflared tunnel route dns GS-1 <application-url>
sudo cloudflared --config .cloudflared/config.yml service install
sudo systemctl start cloudflared
sudo systemctl enable cloudflared

You should now be able to access your device via SSH or SSH in the browser.
``` -->

## 2.4. Setup the Ground Station

Now, we need to clone this repository (specifically the ground station functionality), and begin the setup.

```bash
git clone --no-checkout --depth=1 --filter=tree:0 https://github.com/EricAndrechek/trackuino-v2.git
cd trackuino-v2
git sparse-checkout set --no-cone ground-station
git checkout
cd ground-station
```

Your file structure should now look something like `~/trackuino-v2/ground-station/`.

All that's left is to edit the config.yml file to suit your needs. The below (hidden) section documents in more detail what each section of the config outlines should the config file's comments not be sufficient.

# 3. Additional Walk-Throughs

## 3.1. Config.yml Settings

<details>
    <summary>
        config.yml settings
    </summary>
    TODO: config stuff here
</details>

## 3.2. Web-Interface Setup & Forwarding

# 4. Appendix

Check out the resources below for additional information and readings:

## Libraries

- [librtlsdr](https://github.com/steve-m/librtlsdr)
- [multimon-ng](https://github.com/EliasOenal/multimon-ng)
- [SoX](https://github.com/chirlu/sox)

## References

- [RTL-SDR Blog](https://www.rtl-sdr.com/about-rtl-sdr/)
- [osmosom.org rtl-sdr](https://osmocom.org/projects/rtl-sdr/wiki/Rtl-sdr)
- [Rtl_fm Guide](http://kmkeen.com/rtl-demod-guide/index.html)
- [Radio Reference](https://www.radioreference.com/db/) (useful for seeing nearby frequency allocations)

## SDR Software

Ultimately, just follow this website list and use what works best for you: [rtl-sdr.com/big-list-rtl-sdr-supported-software/](https://www.rtl-sdr.com/big-list-rtl-sdr-supported-software/)

The applications I found the easiest and most useful were:

- [SDR#](https://airspy.com/download/) (Windows)
- [GQRX](http://gqrx.dk/) (Linux/Mac/WSL)
- [SDR++](https://github.com/AlexandreRouma/SDRPlusPlus) (Linux/Mac/Windows)

## Related Works

- [Raspberry Pi + RTL SDR dongle = APRS Rx only gate](https://e.pavlin.si/2014/12/10/raspberry-pi-rtl-sdr-dongle-aprs-rx-only-gate-part-two/) (this exact project minus the packaging, easy setup/config, and web interface)
- [APRS CLI Decoding](https://gist.github.com/jj1bdx/8ab103e774c81d2c068d455ab862b72e) (effectively what this project is doing without a few extra features unqiue to this project)
- [APRS iGate RX from DVB-T tuner](http://sq7mru.blogspot.com/2013/08/aprs-igate-rx-z-tunera-dvb-t.html)
- [pymultimonaprs](https://github.com/asdil12/pymultimonaprs) (very similar, but was python 2, outdated, and didn't have the features we wanted)
- [pymma](https://github.com/ampledata/pymma) (fork of pymultimonapr)
- [aprs-receiver](https://github.com/EricAndrechek/aprs-receiver) (a previous iteration of this project, but for hardware TNCs and built in a rush :sweat_smile:)
