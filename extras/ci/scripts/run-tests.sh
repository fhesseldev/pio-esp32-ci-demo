#!/bin/bash

set -e

# Read hostapd config and add WIFI_PSK and WIFI_SSID defines to the build environment
WIFI_SSID="$(grep ssid /etc/hostapd/hostapd.conf | sed -E 's/ssid=(.*)/\1/')"
WIFI_PSK="$(grep wpa_passphrase /etc/hostapd/hostapd.conf | sed -E 's/wpa_passphrase=(.*)/\1/')"

# Pass the wifi credentials to the Platform IO build process
export PLATFORMIO_BUILD_FLAGS="-DWIFI_SSID=\"\\\"$WIFI_SSID\\\"\" -DWIFI_PSK=\"\\\"$WIFI_PSK\\\"\""

# Serial port on GPIO header of the Rasperry Pi
export PORT="/dev/ttyS0"

# Define some directories
export CIBASEDIR="$(pwd)"
export TESTSDIR="$CIBASEDIR/extras/ci/tests"
export APPSDIR="$CIBASEDIR/extras/ci/apps"

# Start the integration test
python3 -m unittest discover "$TESTSDIR" "*_test.py"
