# pio-esp32-ci-demo

![Continuous Integration](https://github.com/fhessel/pio-esp32-ci-demo/workflows/Continuous%20Integration/badge.svg)

A demo project showing how to setup CI for an ESP32 library on real hardware with GitHub Actions and PlatformIO.

We use a custom action-runner to allow the hardware tests. We configure two workflows:

- [ci-master.yml](.github/workflows/ci-master.yml): Assure that the current master branch can be built.
- [check-pr.yml](.github/workflows/check-pr.yml): An on-demand workflow that checks a PR before merging.

**You will need:**

- An ESP32 development board
- A Raspberry Pi with integrated WiFi (e.g. Model 3B+)
  - Basically, you can use any Linux (ideally Debian) based host with a AP-capable WiFi adapter, but this instruction is tailored to a Pi.
- If your DevBoard has no USB-to-Serial adapter: breadboard and jumper wires

## Setup

We assume you have installed [Raspbian lite](https://www.raspberrypi.org/downloads/raspbian/) on your Pi.

The setup section is structured as follows:

- Wiring between Raspberry Pi and ESP32
- Create user and install required build tools
- Setup WiFi Access Point for the tests
- Setup of GitHub Action Runner
- Creating the actions
- Configuring the repository

### Wiring and Serial Port Configuration

If you're using a DevBoard with USB-to-Serial, plug it in and you're done (continue with user creation and tool installation).

To use a DevBoard that has only the bare ESP32 on it, create the following connections:

| Raspberry Pi | ESP32      | Notes |
| ------------ | ---------- | ----- |
| 5V           | VUSB       | Do not try to power the ESP32 from the 3.3V rail of the Raspberry Pi. It cannot deliver the required current, which may damage your Pi. Instead, either feed the 5V rail into the 5V input, if your dev board has one, or use an external voltage regulator that can deliver enough current like an AMS1117. |
| GND          | GND        |       |
| rx / GPIO15  | tx / GPIO1 |       |
| tx / GPIO14  | rx / GPIO3 |       |
| GPIO17       | EN         |       |
| GPIO18       | GPIO0      |       |

This allows us to flash the ESP32 using [esptool](https://github.com/espressif/esptool) if we manage bootloader and reset manually (RTS and CTS are a bit tricky on the Raspberry Pi).

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

We assume that the [runner is installed as service](https://help.github.com/en/actions/hosting-your-own-runners/configuring-the-self-hosted-runner-application-as-a-service).
Make sure that the `User` entry is set to `esptest` in `/etc/systemd/system/actions.runner.<...>`.

### Configure your Environment

As the configuration of the ESP32 board (serial port, board type, ...) are not repository-specific but runner-specific, we do not configure them in the workflow but on the local machine.

All of the following settings are optional, they default to using a WROVER board with 16MB flash and the GPIOs to communicate with the ESP32.
If you want to enable them, enter them in the `[Service]` section of `/etc/systemd/system/actions.runner.<...>`.

**Other board**: This setting relates to the PlatformIO build environment that is used to build the applications during hardware test. See [`extras/ci/apps/chat-server/platformio.ini`](extras/ci/apps/chat-server/platformio.ini) for the differences.

```
Environment="PIOENV=wroom"
```

**Serial Port**: Use a different serial port. Defaults to `/dev/ttyS0`

```
Environment="PORT=/dev/ttyUSB0"
```

**Reset Method**: Use esptool instead of GPIO reset (required if you connect a DevBoard with integrated reset circuit, you most likely need to change the serial port as well)

```
Environment="ESPRESETTOOL=esptool"
```

### Creating the Actions

> **Caution:** Be careful when adding actions with a self-hosted runner.
If you allow actions to be triggered by any person and to run on a branch in control of that person, you will face a remote code execution vulnerability on your runner.
[GitHub does not recommend this](https://help.github.com/en/actions/hosting-your-own-runners/about-self-hosted-runners#self-hosted-runner-security-with-public-repositories), and I do not guarantee the proposed setup to be secure.
If you identify a problem with the given configuration, please let me know.

For the reason above, please make sure that only you and people you trust have write access to your repository.
With that assumption, a `push` workflow on the master branch should be safe.
For the pull request workflow, we will define a label that triggers the CI build, and which has to be set manually **after a thorough review** of the incoming changes (more on that below).

#### Workflow Steps

Both workflows contain of two steps:
1. Building all examples in the `examples/` folder to verify that they're still working.
2. Running all Python unit tests in `extras/ci/tests` against applications in `extras/ci/apps`.

**Building Examples:** This step runs [`extras/ci/scripts/build-examples.sh`](extras/ci/scripts/build-examples.sh).
The script iterates over all directories in `examples/` that have a `.ino` file in them with the same name as the directory (that is basically what the Arduino IDE does to find examples).
Then, it creates a temporary directory for the example, and ...

- Puts all files from the example directory in a `src` subfolder
- Renames the `.ino` file to `main.cpp` and includes `Arduino.h` at its top
- Places the `example-platformio.ini` file from the `templates` directory as `platformio.ini` in that folder
- Creates a `lib` directory and puts a symlink to the library in it

This creates a PlatformIO project folder from the example, which then can be built. If building of all examples succeeds, this step succeeds.

**Running Hardware Test:** This step runs [`extras/ci/scripts/run-tests.sh`](extras/ci/scripts/run-tests.sh).
The script uses Python's [`unittest`](https://docs.python.org/3/library/unittest.html) library to build integration tests against the hardware module.
It will execute all test cases in files named `*_test.py` under `extras/ci/tests`.

The `testhelper.py` contains a base class which resets the ESP32 for each individual test and flashes a specific application to it.
Use it like this:

```python
from testhelper import ESPTestCase, appname
@appname("chat-server")
class MyTestCase(ESPTestCase, unittest.TestCase):
  pass
```

It then will flash the annotated application from the `extras/ci/apps` folder before the test and then reboot the ESP32 before each test-method in that class.

The applications in `extras/ci/apps` are default Platform IO projects which are built on demand with the current version of the library.
You can use `WIFI_SSID` and `WIFI_PSK` macros in them, which will be set according to the current hostapd config file.
The test runner expects the ESP32 to connect to the WiFi within 15 seconds.
You also need to print a small JSON status to the serial output to tell the host that you're ready: `{"event":"connected","ip":"192.168.42.2"}`.
Check the comments in the example application for more details on the interplay of the test suite and the application.

#### Workflows

We use the steps to create our two workflows:

**Master Workflow:** The master workflow is quite simple.
It uses the common checkout action to get a copy of the repository.
Then, it will build the examples.
If that succeeds, it runs the hardware-based tests and after that reports the status back to the commit.
The trigger is any push to the master branch.

**PR Workflow:** Currently, it is not possible to trigger a workflow manually on a commit.
Therefore, we need to create a workaround that allows us to review a PR before we pass it to the runner, to prevent the previously menitoned security issues.
We exploit issue lables to reach that goal.
This workflow does the same as the master workflow, but is triggered when a label is set on a pull-request and the `CI - Ready to run` label (make sure to create that label, with this exact name) on that pull request is set.
While *Pull-Request* and *label added* are things which can be checked on workflow level, the label list has to be checked in each step.
Furthermore, it is not possible to know *which* label was added.
Therefore, and to allow re-checking a PR after changes, we add an additional step that removes the label just after the workflow has started.
This way, the CI runs for latest commit in the PR and only that commit will be marked as valid.
Also, adding labels is an action which can only be done by known users, which makes this action as secure as it can get.

You should check the PR twice before adding the CI label, if ...
- The `.github/workflows` folder is modified
- The `extras/ci` folder is modified
  - In particular, if the `scripts` subfolder is modified
  - New tests also have to be revised. Python allows easy integration testing of, e.g., a REST APIs on the ESP32, but is powerful and is not limited to communication with your ESP32 but can do anything on the system.

### Configuring the Repositry

If you want to assure that only a CI-checked PR can be merged, you can create a branch protection rule.

Go to *Settings* → *Branches* → *Branch Protection Rules* and click on *Add rule*.

Then configure the rule like shown in this screenshot.

> **Note:** You may need to create a dummy PR and label it for all options to appear here.

![Branch Protection Rule](https://github.com/fhessel/pio-esp32-ci-demo/blob/master/extras/assets/branch-protection.png?raw=true)

Now, every PR should require the PR-Workflow to run before merging.
You can trigger the workflow by adding the `CI - Ready to run` label, the Actions Bot will remove it again.
