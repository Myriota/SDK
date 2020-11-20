# Changelog

## 1.4.0

API changes:

- Adds APIs to support suspend mode
- Adds API MessageQueueClear to clear message queue
- Adds pull option to pulse counter pin in PulseCounterInit API
- Changes the TX on duration of RF test TX API
- Changes return value of ScheduleMessage
- Deprecates DUMPTX environment variable and replaces it with SATTEST

Improvements:

- Adds suppend mode support
- Adds an option to updater.py to dump device debug output
- Reduces module downlink energy consumption
- Improves updater.py and log-util.py to automatically detect the serial port to use
- Improves i2c_spi example to set SPI baud rate accurately

Fixes:

- Fixes the Band parameter to BoardAntennaSelect BSP API when the module is receiving
- Fixes a baud rate accuracy issue in SPI API when SPI_BAUDRATE_MAX is used

## 1.3.2

Improvements:

- Adds development board v2 support
- Adds modem with AT commands support example
- Adds default bsp file to all examples
- Adds documentation to RF test examples
- Reduces current consumption of Sleep API
- Adds an example to test GNSS chip
- Renames terminal folder to module
- Adds versioning support to network information

Fixes:

- Fixes an issue where the module periodically wakes up for a very short period of time if LEUART or pulse counter remains enabled in sleep mode
- Fixes certificate deletion issue in network information

## 1.3.1

Fixes:

- Updates network information revision

Improvements:

- Improves LEUART and RF test examples

## 1.3.0

API changes:

- Adds API for scaling the message throughput based on current network
- Adds API to return the number of bytes free in the message queue

Improvements:

- Increases robustness to service outage
- Adds support to update network information via updater.py
- Improves Sense&Locate example
- Reformats Python tools

## 1.2.2

Fixes:

- Fixes an issue where `BeforeSatelliteTransmit` may not return the correct time after network information update

## 1.2.1

API changes:

- Deprecates ScheduleMessage return value

Improvements:

- Adds example code of Myriota's Sense&Locate board
- Adds API to get RSSI reading for RF test
- Increases rate of diagnostics messages
- Adds BSP API to support sleep hooks
- Adds folder structures under application folder for generated files
- Adds baudrate options to updater.py
- Improves compatibility of updater.py
- Adds more options to satellite_simulator.py
- Improves python3 support of log-util.py

## 1.2.0

Fixes:

- Fixes an issue where the module may not sleep properly
- Fixes an issue where user job may hang after disabling debug output

 Improvements:

- Adds OTA network updates support
- Adds more system diagnostics
- Adds hardware watchdog support
- Adds API to get registration code
- Updates updater.py to support new flash layout

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
