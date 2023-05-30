# Tracking Dashboard

This is an open-source application to either pull data from APRS-IS or from your own database and API endpoint for tracking your balloons and vehicles.

This requires a server to host this webpage and its database.

Additionally, this tool assumes your ground stations were set up using Cloudflare Tunnels for tracking their uptime.

This tool is cloud agnostic, although I highly recommend [Oracle's "always free" tier](https://www.oracle.com/cloud/free/) as it has the needed compute and database storage for everything we need to do.

# Setup VM

Go to [Oracle's "always free" tier](https://www.oracle.com/cloud/free/) and create an account. Create a compute instance and fill it out so that you are still within the free tier. When you get to the boot volume settings, set the storage to 200GB, as that is the max you are allowed on the free tier.

Next, SSH into your server. It is recommended that you run `sudo passwd ubuntu` and set a password for the default user. Then edit the SSH config by running `sudo nano /etc/ssh/sshd_config` and changing `PasswordAuthentication no` to `PasswordAuthentication yes`. Now if you'd like to allow other to SSH in, you can share the password with them without worrying about SSH keys. (This is less secure).

Optionally, to improve the security of your system, once you have it set up follow the same steps as in the Ground Station tutorial to add a Cloudflare Tunnel to your system. This will also allow you to monitor and ssh into your server from the same place as your ground stations.

# Setup DB

We will use a simple SQLite database on the VM. 

# Clone Repo

```bash
git clone --no-checkout --depth=1 --filter=tree:0 https://github.com/EricAndrechek/trackuino-v2.git
cd trackuino-v2
git sparse-checkout set --no-cone tracking-dashboard
git checkout
cd tracking-dashboard
```
