# pio-esp32-ci-demo

A demo project showing how to setup CI for an ESP32 library on real hardware with GitHub Actions and PlatformIO.

**You will need:**

- An ESP32 development board
- A Raspberry Pi
  - A model with WiFi support (e.g. the 3B+) if you want to test things via WiFi
- Breadboard and jumper wires

## Setup

We assume you have installed [Raspbian lite](https://www.raspberrypi.org/downloads/raspbian/) on your Pi.

The setup section is structured as follows:

- Wiring between Raspberry Pi and ESP32
- Create user and install required build tools
- Setup WiFi Access Point for the tests
- Setup of GitHub Action Runner
- Creating the actions

### Wiring and Serial Port Configuration

Create the following connections:

| Raspberry Pi | ESP32      | Notes |
| ------------ | ---------- | ----- |
| 5V           | VUSB       | Do not try to power the ESP32 from the 3.3V rail of the Raspberry Pi. It cannot deliver the required current, which may damage your Pi. Instead, either feed the 5V rail into the 5V input, if your dev board has one, or use an external voltage regulator that can deliver enough current like an AMS1117. |
| GND          | GND        |       |
| rx / GPIO15  | tx / GPIO1 |       |
| tx / GPIO14  | rx / GPIO3 |       |
| GPIO17       | EN         |       |
| GPIO18       | GPIO0      |       |

This allows us to flash the ESP32 using [esptool](https://github.com/espressif/esptool) if we manage bootloader and reset manually.

The complete setup could look like this (bare WROVER board with external voltage regulator):

![Setup on breadboard](https://github.com/fhessel/pio-esp32-ci-demo/blob/master/extras/assets/breadboard-setup.jpg?raw=true)

If not done yet, run `sudo raspi-config`, go to the peripherals section, disable the login shell on the serial port.
When asked if the serial port should be made available, select *yes* and reboot.
Now, you can access the serial port your ESP32 on `/dev/ttyS0`, e.g. by running `miniterm /dev/ttyS0 115200`.

### Create User and Install Tools

Install python, support for virtual environments, ...

```bash
sudo apt-get update
sudo apt-get -y install python3 python3-venv python3-serial
```

We will install all tools that do not come with the package manager in the home directory of the `esptest` user.
Create it with the following command:

```bash
sudo useradd -m -s /bin/bash -G dialout,plugdev,gpio,spi,i2c,plugdev,netdev esptest
```

You can now get as shell for this user by running:

```bash
sudo -u esptest bash
```

The basic skeleton of the home directory's contents is available under `extras/setup/esptest`.

Copy it to the files from the repository to the home directory of that user (caution: also the hidden files like .bashrc).
Then run `setup.sh` to create virtual environments and download additional software.

### Setup WiFi Access Point for the Tests

> **Note:** This is a short version of the description in the [official docs](https://www.raspberrypi.org/documentation/configuration/wireless/access-point.md).

Install `hostapd` (wireless access point), `dnsmasq` (DHCP server) and `pwgen` (used for key generation). Use a sudo-user for this, not your `esptest` user:

```bash
sudo apt-get update
sudo apt-get -y install pwgen hostapd dnsmasq
```

Stop the services to configure them:

```bash
sudo systemctl stop dnsmasq
sudo systemctl stop hostapd
```

Check the name of your WiFi interface. Run the following command ...

```bash
ip addr
```

... and check for a line like this ...

```
3: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN group default qlen 1000
```

... where `wlan0` is the interface name you need. Then edit `/etc/dhcpcd.conf` and add:

```
interface wlan0
    static ip_address=192.168.42.1/24
    nohook wpa_supplicant
```

Make sure to use the interface name that you've found before and use an IP address that is not in use in your local network.
If you pick a different address (not `192.168.42.1`), make sure to replace it in the following files.

Replace `/etc/dnsmasq.conf` by the following file (or copy it from [here](extras/setup/dnsmasq.conf)):

```
interface=wlan0
dhcp-range=192.168.42.2,192.168.42.20,255.255.255.0,24h
```

Copy the sample [`hostapd.conf`](extras/setup/hostapd.conf) file to `/etc/hostapd/hostapd.conf` and make sure to set a secure password.
You can create one by running:

```bash
pwgen -s 63 1
```

Tell the system about the configuration file by editing `/etc/hostapd/hostapd` and adding this line:

```
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

Now start everything:

```bash
sudo systemctl restart dhcpcd
sudo systemctl start dnsmasq
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl start hostapd
```

This is sufficient to test communication between your ESP32 and the Raspberry Pi.
If you, however, need outbound traffic from the ESP32 to the Internet or another network, you need to follow the steps under *Add routing and masquarade* and *Using the Raspberry Pi as an access point to share an internet connection (bridge)* in the [Access Point on Raspberry Pi Documentation](https://www.raspberrypi.org/documentation/configuration/wireless/access-point.md).

### Setup GitHub Action Runner

Open your repository in GitHub, go to *Settings* → *Actions* and click on *add runner* in the *Self-Hosted Runners* section.

Select *ARM* as architecture and follow the instructions below.

Make sure to log in as the `esptest` user, and create the folder in the first step as `/home/esptest/apps/actions-runner`.

More information is available [on GitHub Help](https://help.github.com/en/actions/hosting-your-own-runners).
