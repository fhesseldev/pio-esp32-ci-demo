# Continuous Integration

This directory contains the tests that are executed for each commit/PR/...

There are two subfolders:

- `apps` contains the PlatformIO applications that are tested
- `scripts` contains scripts that are called during the build process
- `templates` contains files required for the build process, e.g. to create full project folder from Arduino examples.
- `tests` contains the test suites and test cases that are executed on the Raspberry Pi.
