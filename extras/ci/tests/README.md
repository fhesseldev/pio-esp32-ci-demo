# Integration Tests

We use Python's [`unittest`](https://docs.python.org/3/library/unittest.html) library to define integration tests that check the external interface of the ESP32.

Each `*_test.py` file in this directory can contain a test case. Each of the cases uses the `@appname` annotation to select an application from the `ci/apps` folder that will be uploaded to the ESP32 before the test starts.

The `testhelper.py` file contains a base class to ease handling the hardware.

`multiuser_test.py` runs three tests, first just to connect to the server, then to create five connections simultaneously, and finally message passing between to clients in both directions.
