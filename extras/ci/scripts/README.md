# Scripts

Contains scripts that are used in the jobs under `.github/workflows`.

As we run the scripts on a Linux host, we can just use bash scripts and do not need the abstraction layer of Docker or JS/TS actions.

| script | workflow | description |
| ------ | -------- | ----------- |
| build-examples.sh | build-examples | Copies all examples from `examples/` to PlatformIO project directories under `.tmp/` and tries to build them (without flashing to the chip) to make sure none of the examples is broken by the latest commit |
| run-tests.sh | run-tests | Runs the integrations tests in the `ci/tests` folder against the apps in `ci/apps` |
