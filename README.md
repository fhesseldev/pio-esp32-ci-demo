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
- Install helpful tools
- Installation of Platform IO
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

### Install Tools

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

### Setup GitHub Action Runner

Open your repository in GitHub, go to *Settings* → *Actions* and click on *add runner* in the *Self-Hosted Runners* section.

Select *ARM* as architecture and follow the instructions below.

Make sure to log in as the `esptest` user, and create the folder in the first step as `/home/esptest/apps/actions-runner`.

More information is available [on GitHub Help](https://help.github.com/en/actions/hosting-your-own-runners).
