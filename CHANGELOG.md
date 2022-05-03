# Changelog

## 1.5.6

Improvements:
  - Adds support to retrieve network information and diagnostics data from the module. The data can be uploaded to Myriota cloud for post-processing

## 1.5.5

Fixes:

- Fixes an issue where the wakeup capability of both GPIOs is disabled when `GPIODisableWakeup` is called
- Fixes an issue in `RFTestTxStop` hardware API where LED won't be turned off when the test stops
- Fixes an issue in the RSSI test of the `rf_test` example where the BSP antenna selection logic is not been used properly
- Fixes an issue in the makefile of `i2c_spi` example where SPI mode binary cannot be built
- Fixes an issue where unnecessary diagnostic messages are generated when specific network information is received via downlink

Improvements:

- Adds APIs to update the network information in the user application
- Significantly reduces the startup time after programming the user application or network information
- Reduces the wakeup time when neither LEUART nor pulse counter is enabled
- Avoids potential current leak from GNSS UART1_RX pin in deep sleep
- Increases user job watchdog timeout time from 6 minutes to 15 minutes

## 1.5.4

Improvements:

- Optimises TX scheduling to improve TX performance
- Adds support of applying TX calibration table in the module to tune the TX power and current draw
- Clarifies temperature sensor readings in the comment/document
- Improves satellite simulator to capture non-real-time packets
Fixes:

- Corrects module TX frequency selection error which reduces the message success rate
- Fixes an issue where SDK 1.5.3 is not compatible with applications built against older SDKs

## 1.5.3

Improvements:

- Adds SDK APIs to save user data in [persistent storage](examples/nvram/README.md)
- Adds SDK API `RxStatsGet` to get [downlink stats](examples/rf_test/rx_packet/README.md)
- Improves the support of network performance tuning via network information
- Improves the Sense&Locate [example](examples/snl) to simplify sensor testing
- Adds permission for logging to unpacker example for the [tracker example application](examples/tracker/aws)
- Adds support for macOS installation
- Improves the accuracy of the GNSS fix when the device is moving
- Adds an option to `mergy_binary.py` tool to extract individual raw files from a merged file
- Stops printing satellite IDs to the debug output
- Removes obsolete functions from the math folder

Fixes:

- Corrects rare event causing module to reset during satellite prediction

## 1.5.2

API changes:

- Changes the `GNSSFIX` environment variable for satellite simulator mode (lab mode)
- Network information and user application binaries for programming via XMODEM tool or SWD hardware interface are generated under the `raw_binary` folder

Improvements:

- Adds support to separate the network information from the user application
  - the user application can fully utilise the 34k-byte flash memory allocated to it
  - network information and user application can be updated individually during development and manufacturing
- Adds `merge_binary.py` tool to merge multiple firmware files into one
  - multiple different types of files can be merged into one and updated by `updater.py` with `-m` option
  - file contents are CRC checked to detect corruption
  - the network information and the user application are merged together by default and can be updated by either `-u` for backward compatibility or `-m`
- Adds support to print memory usages when building the user application
- Improves `message_inject.py` error handling when the module ID specified does not exist
- Adds CloudFormation template of an AWS lambda for the tracker example application
- System GNSS update job won't run when system GNSS fix is skipped to reduce module wakeups
- Adds redirection support for network information download to be future-proof
- Improves `log-util.py` error handling when log purging fails

Fixes:

- Fixes an issue in `BeforeSatelliteTransmit` where time based on an invalid transmit opportunity may be returned
- Fixes an issue in `updater.py` where failed communication during updating may be recognised as partition error
- Fixes an issue in `log-util.py` when decoding user log entries with zero length
- Fixes an issue where suspend mode can't be disabled in `BoardStart`
- Fixes an issue when the system GNSS is skipped causing delayed diagnostic messages
- Fixes an issue when the system GNSS is skipped the location set is reset to default after disabling suspend mode

## 1.5.1

Fixes:

- Fixes the hard fault when no user job is scheduled and suspend mode is enabled
- Fixes an issue where impending satellite passes within 10 minutes won't be returned from BeforeSatelliteTransmit or predicted after initial GNSS fix

Improvements:

- Increases TX rate to improve network uplink capacity
- Improves handling of enabling suspend mode in BoardStart
- Improves handling of scheduling user jobs in BoardStart
- Adds unpacker example for the SnL example application

## 1.5.0

API changes:

- Returns error from `ScheduleMessage` if a message of zero length is scheduled
- Reduces the flash size available to the application
- Floating point scanf support in the application needs to be enabled by `SCANF_FLOAT = 1` makefile variable

Other changes:

- Drops installation support on Ubuntu 16.04 OS

Improvements:

- Reduces radio receive energy consumption
- Improves diagnostics
- Improves Sense&Locate example
- Adds modem hardware test support to at_modem example
- Adds debug option to updater.py
- Improves error report of updater.py
- Reduces size of libraries for building applications

## 1.4.1

API changes:

- Adds API `I2CInitEx` to support different bus speeds

Fixes:

- Fixes watchdog timeout issue when doing GNSS fix
- Fixes sigpipe exception when running updater.py under Windows
- Fixes user job run count in the log
- Fixes an issue where API MicroSecondDelay does not handle 0 as input parameter properly

## 1.4.0

API changes:

- Adds APIs to support suspend mode
- Adds API `MessageQueueClear` to clear message queue
- Adds pull option to pulse counter pin in `PulseCounterInit` API
- Changes the TX on duration of RF test TX API
- Changes return value of `ScheduleMessage`
- Deprecates DUMPTX environment variable and replaces it with SATTEST

Improvements:

- Adds suspend mode support
- Adds an option to updater.py to dump device debug output
- Reduces module downlink energy consumption
- Improves updater.py and log-util.py to automatically detect the serial port to use
- Improves i2c_spi example to set SPI baud rate accurately

Fixes:

- Fixes the Band parameter to `BoardAntennaSelect` BSP API when the module is receiving
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
- Additional module frequency capabilities

## 1.2.2

Fixes:

- Fixes an issue where `BeforeSatelliteTransmit` may not return the correct time after network information update

## 1.2.1

API changes:

- Deprecates `ScheduleMessage` return value

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

- Changed the input paramter of `BoardAntennaSelect` to enable more control to the antenna select pin
- Adds support to wake up from LEUART interface and corresponding example code
- Adds TimeSet to set the system time
- Adds LocationSet to set the current location
- Adds ModuleIDGet to get module ID string
- Adds API to support satellite radio RF test
- Adds BSP API `BoardDebugRead`

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
