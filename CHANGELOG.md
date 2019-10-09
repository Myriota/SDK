# Changelog

## 1.1.4

Fixes:

- Fixes an issue where LEUART interface may lose characters if deinitialised straightaway after write

Improvements:

- Adds API to save user messages to continue transmission after reset
- Adds support to recover scheduled user messages when the system resets due to unrecoverable failures
- Adds support to send diagnostics messages when the system resets due to unrecoverable failures
- Adds support to log the PC address where watchdog timeout occurs
- Adds logging of reset reason
- When GNSSFIX is set to 1, GNSS fix only gets time after location has been fixed or set
- Disables the radio completely when calling `RFTestTxStop`
- Adds API to do micro-second delay
- Adds dumping of module ID to the log
- Removes `make run` build target
- Cleans up development board BSP BoardBatteryVoltGet implementation
- Adds support to include custom BSP (`bsp.c`) from application folder

## 1.1.3

API changes:

- Reduces I2C clock rate to be compatibility slow devices

Improvements:

- Adds tracker web application example
- Improves error handling in Python tools

## 1.1.2

Fixes:

- Fixes an issue where TX power is low when building from SDK 1.1.1

API changes:

- ModuleIDGet returns module part number as well


Improvements:

- Adds support to dump encoded messages for injection when running in LabWithLocation mode
- Adds support of revision 4 modules

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
