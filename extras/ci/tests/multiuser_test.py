import unittest
import socket
import time
import sys

from testhelper import ESPTestCase, appname

# This annotation references an app in the apps folder, which will be uploaded to the
# device under test before the tests are run.
@appname("chat-server")
class MultiUserTestCase(ESPTestCase, unittest.TestCase):

  def test_connect(self):
    """
    Test that a client can connect to the chat server
    """
    # self._esp_ip contains the ESP's IP address after a WiFi connection has been established.
    # The test app needs to send a log line like this ...
    #   {"event":"connected","ip":"10.20.30.40"}
    # which is then picked up by the ESPTestCase class.
    addr = socket.getaddrinfo(self._esp_ip, 1337, socket.AF_INET, socket.SOCK_STREAM)[0]
    (family, socktype, proto, canonname, sockaddr) = addr
    with socket.socket(family, socktype, proto) as s:
      s.connect(sockaddr)
      time.sleep(1)

  def test_multiconnect(self):
    """
    Test that five clients can simultaneously connect to the chat server
    """
    addr = socket.getaddrinfo(self._esp_ip, 1337, socket.AF_INET, socket.SOCK_STREAM)[0]
    (family, socktype, proto, canonname, sockaddr) = addr
    sockets = []
    try:
      for n in range(5):
        s = socket.socket(family, socktype, proto)
        s.connect(sockaddr)
        sockets.append(s)
      time.sleep(2)
    finally:
      e = None
      for s in sockets:
        try:
          s.close()
        except:
          e = sys.exc_info()[1]
      if e is not None:
        raise e

  def test_chat(self):
    """
    Test for message passing between two clients in both directions
    """
    addr = socket.getaddrinfo(self._esp_ip, 1337, socket.AF_INET, socket.SOCK_STREAM)[0]
    (family, socktype, proto, canonname, sockaddr) = addr
    with socket.socket(family, socktype, proto) as alice, \
        socket.socket(family, socktype, proto) as bob:
      alice.connect(sockaddr)
      bob.connect(sockaddr)
      self.assertEqual(8, alice.send(b'Hi, Bob\n'), "Alice couldn't send all bytes")
      time.sleep(1)
      self.assertEqual(b'Hi, Bob\n', bob.recv(100), "Bob didn't get Alice's message")
      self.assertEqual(10, bob.send(b'Hi, Alice\n'), "Alice couldn't send all bytes")
      time.sleep(1)
      self.assertEqual(b'Hi, Alice\n', alice.recv(100), "Alice didn't get Bob's message")
