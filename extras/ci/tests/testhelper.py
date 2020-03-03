import unittest
import serial
import sys
import os
import subprocess
import threading
import time
import json

def appname(appname):
  def apply_appname(clazz):
    clazz.appname = appname
    return clazz
  return apply_appname

INSTALLED_APP = None

class ESPTestCase(unittest.TestCase):

  def esp_install_app(self, appname):
    """ Installs an app to the ESP32 """
    self._appdir = os.path.join(self._appsdir, appname)
    libdir = os.path.join(self._appdir, 'lib')
    if not os.path.isdir(libdir):
      os.mkdir(libdir)
    liblink = os.path.join(libdir, 'cilib')
    if not os.path.islink(liblink):
      os.symlink(os.environ['CIBASEDIR'], liblink, True)
    self.esp_reset(True)
    print("Uploading %s to %s" % (appname, self._port))
    upload = subprocess.Popen([
      "pio", "run",
      "-t", "upload",
      "-d", self._appdir,
      "--upload-port", self._port])
    res = upload.wait()
    self.assertEqual(res, 0, "Could not install %s" % appname)
    self.esp_reset(False)

  def esp_reset(self, enter_bootloader=False):
    """ Resets the ESP32 """
    reset = subprocess.Popen(["espreset",
      "bootloader" if enter_bootloader else "reset"])
    res = reset.wait()
    self.assertEqual(res, 0,
      "Entering bootloader failed" if enter_bootloader else "Reset failed")

  def setUp(self):
    global INSTALLED_APP
    self._port = os.environ['PORT'] if 'PORT' in os.environ else '/dev/ttyUSB0'
    self._appsdir = os.environ['APPSDIR']
    self._wifi_ready = threading.Event()
    if INSTALLED_APP != self.appname:
      self.esp_install_app(self.appname)
      INSTALLED_APP = self.appname
    self._serial = serial.Serial(self._port, 115200, timeout = 0.1)
    print("ESP32: <opened serial connection on %s>" % self._port)
    self._esp_ip = None
    self._serial_active = True
    self._serial_thread = threading.Thread(target=self._serThread,daemon=True)
    self._serial_thread.start()
    time.sleep(0.5)
    self.esp_reset(False)
    self.assertTrue(self._wifi_ready.wait(15),
      "Did not get a WiFi connection in 15 seconds")
    time.sleep(3)

  def tearDown(self):
    # Stop the thread
    t = self._serial_thread
    if t is not None:
      self._serial_active = False
      t.join()
      self._serial_thread = None
    s = self._serial
    if s is not None:
      try:
        s.close()
      except:
        pass
      self._serial = None

  def _serThread(self):
    try:
      resetcount = 0
      linebuf = b''
      while self._serial_active:
        line = linebuf + self._serial.readline()
        if line != b'':
          if not line.endswith(b'\n'):
            if len(line) < 4096:
              linebuf = line
              continue
          else:
            linebuf = b''
          if line.startswith(b'rst:0x'):
            resetcount += 1
          decoded_line = line.decode('utf8',errors='ignore').strip()
          if resetcount < 20:
            print("ESP32: " + decoded_line)
            try:
              data = json.loads(decoded_line)
              if 'event' in data and data['event'] == 'connected':
                print("ESP32: <WiFi connected>")
                self._esp_ip = data['ip']
                self._wifi_ready.set()
            except json.JSONDecodeError:
              pass
          elif resetcount == 20:
            print("ESP32: <bootloop, skipping output>")
        else:
          time.sleep(0.05)
          resetcount = max(0, resetcount - 3)
    finally:
      print("ESP32: <serial connection closed>")
      self._serial_active = False
      self._serial_thread = None
