# Changelog

## 1.1.1

API Changes:

- Changed the input paramter of BoardAntennaSelect to enable more control to the antenna select pin
- Adds support to wake up from LEUART interface and corresponding example code
- Adds TimeSet to set the system time
- Adds LocationSet to set the current location
- Adds ModuleIDGet to get module ID string
- Adds API to support satellite radio RF test
- Adds BSP API BoardDebugRead

Improvements:
- Improves the pulse counter example code
- Improves updater.py to support waiting for serial port to be ready

## 1.1.0

API changes:

- Adds pulse counter support
- XXXInit returns failure if called more than once without XXXDeinit
- Deprecates `--username` argument for CLI tools. See "CLI tools cache a token" below

Improvements:

- CLI tools cache a token and so request username and password less frequently
- Adds support to skip GNSS fix for test
- Fixes issue where GPIO events generated during job execution may get missed
- Adds message_inject.py tool to programatically send test messages
- Adds better support/documentation for testing in an indoor lab environment
- Adds support to build applications offline
- Consolidates header files and cleans up inline documents

## 1.0.0

Initial release
