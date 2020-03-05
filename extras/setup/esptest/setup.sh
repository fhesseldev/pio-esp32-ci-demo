#!/bin/bash

set -e

mkdir -p /home/esptest/bin
mkdir -p /home/esptest/apps

echo "Installing esptool"
mkdir -p /home/esptest/apps/esptool
cd /home/esptest/apps/esptool
python3 -m venv env
source env/bin/activate
pip install esptool
deactivate
ln -s /home/esptest/apps/esptool/env/bin/esptool.py /home/esptest/bin/esptool
ln -s /home/esptest/apps/esptool/env/bin/espfuse.py /home/esptest/bin/espfuse
ln -s /home/esptest/apps/esptool/env/bin/espsecure.py /home/esptest/bin/espsecure

echo "Installing platformio"
mkdir /home/esptool/apps/platformio
cd /home/esptool/apps/platformio
python3 -m venv env
source env/bin/activate
pip install platformio
deactivate
ln -s /home/esptest/apps/platformio/env/bin/platformio /home/esptest/bin/platformio
ln -s /home/esptest/apps/platformio/env/bin/pio /home/esptest/bin/pio
ln -s /home/esptest/apps/platformio/env/bin/piodebuggdb /home/esptest/bin/piodebuggdb
