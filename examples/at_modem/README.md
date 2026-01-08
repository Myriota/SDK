# AT Command Modem Example

## Hardware connection

 - LEUART Tx/Rx
 - GND
 - GPIO7(optional, outputs high to indicate a job is running)

## Serial communication configuration

  - Baudrate: 9600
  - Data bits: 8
  - Stop bits: 1
  - Parity: None
  - Flow control: None

## Communication protocol

- Commands start with prefix `AT`
- Commands are case sensitive. All characters in commands should be in upper case
- All white spaces such as `<CR><LF>`, `<LF>` and Space in a command are recognized as command terminators
- Responses end with `<CR><LF>`
- `<CR>` stands for carriage return character. ASCII value is 13
- `<LF>` stands for line feed character. ASCII value is 10
- Maximum command length is 80 characters including prefix and terminators
- Commands have 2 types - query and control
- All commands have a response

### Power up

- Modem sends `+STATE=INITIALIZING` when the system is initializing
- Modem sends `+STATE=GNSS_ACQ` if the system failed to get initial GNSS fix acquisition and starts continual retrying automatically
- Modem sends `+STATE=READY` if the system has acquired a GNSS fix
- The state changes to READY once a GNSS fix has acquired which can be queried by the `STATE` command
- Only in the READY state can a scheduled message be transmitted

### Communication check

- Command

    `AT`

- Response

    `OK`

### Queries

- Command

    `AT+<CMD>=?`

- Response

    Success: `OK+<CMD>=<VALUE>`

    Failure: `FAIL+<CMD>=<VALUE>`

- Query command list

    | Command           | Command string | Return              | Note                        |
    |-------------------|----------------|---------------------|-----------------------------|
    | Message queue     | MSGQ     | Number of free slots in the message queue | - |
    | System state      | STATE    | INITIALIZING, GNSS_ACQ or READY | Refer to power up logic |
    | SDK version       | VSDK     | SDK version | Format: major.minor.patch |
    | Module ID         | MID      | Module ID and part number | E.g. 0012345678 M1-24 |
    | Registration code | REGCODE  | Registration code | - |
    | Get time          | TIME     | Unix epoch time | E.g. 1672531200 |
    | Get location      | LOCATION | Latitude and longitude of last GNSS fix, scaled by 1e7 | E.g. -349205499,1386086737 |
    | Get suspend mode  | SUSPEND  | 0: disabled, 1:enabled | - |
    | Message queue status | MSGQS | Transmission status of messages in the message queue | - |

### Controls

- Command

    Without parameter: `AT+<CMD>`

    With parameter: `AT+<CMD>=<PARAMETER>`

- Response

    Success: `OK+<CMD>` or `OK+<CMD>=<PARAMETER>`

    Failure: `FAIL+<CMD>` or `FAIL+<CMD>=<PARAMETER>`

- Control command list

    | Command               | Command string | Parameter           | Note                        |
    |-----------------------|----------------|---------------------|-----------------------------|
    | Save message queue    | SAVEMSG  | N/A | Save message queue before hardware reset or power cycle |
    | Start RF TX test      | TXSTART  | Refer to RF TX parameter below | Return "OK+TXSTART=PARAMETER" first, then return "OK+TXSTART" after timeout |
    | Stop RF TX test       | TXSTOP   | N/A | - |
    | Start GNSS fix        | GNSSFIX  | N/A | Return OK immediately then return OK again when successful or return FAIL after 90s timeout |
    | RSSI test             | RSSI     | Frequency in Hz. E.g. 400000000 | Return "OK+TXSTART=RSSI" in dBm on the specified frequency |
    | Schedule message      | SMSG     | Hex string of the message | The length should be even |
    | Change suspend mode   | SUSPEND  | 1 to enable and 0 to disable | - |
    | Set time              | TIME     | Unix epoch time | E.g. 1715584647 |
    | Set location          | LOCATION | Latitude and longitude to be set, scaled by 1e7 | E.g. -349205499,1386086737 |
    | Delete message from queue | MSGQD    | Deletes a message from the queue by its message ID | - |

- RF TX parameter

    Format: `<FREQUENCY>,<TX_TYPE>,<BURST_MODE>,<TIMEOUT>`

    E.g. AT+TXSTART=161450000,0,1,60

    | Parameter  | Values    |
    |------------|-----------|
    | Frequency  | In Hz. E.g. 161450000 / 400000000 |
    | TX type    | 0 - Tone(CW), 1 - PRBS Mode |
    | Burst mode | 0 - Continuous Mode, 1 - Burst Mode |
    | Timeout    | 0 to 999 in seconds |

### Error codes

| Error code         | Meaning                             | Countermeasure                |
|--------------------|-------------------------------------|-------------------------------|
| INVALID_PARAMETER  | Control commands are carrying illegal parameters | Debug port(UART0 - 115200,N,8,1) can be used to monitor detail causes when debugging |
| MESSAGE_TOO_LONG   | Scheduled message is too long | Reduce message size |
| TOO_MANY_MESSAGES  | Too many messages scheduled in an hour | Wait for sometime before scheduling the message |
| BUFFER_OVERFLOW    | Modem RX buffer overflow | Reduce UART frame length |
| UNKNOWN_QUERY_CMD | Query identifier "=?" detected but no command match | Check query command list |
| UNKNOWN_CONTROL_CMD | Control format matched but no command is found | Check control command list |
| INVALID_COMMAND    | Command format error, can be caused by prefix/terminator match or command/parameter too long | Check command format or monitor debug port for detail reason |

### Timeout and retry

The timeout time of modem responses is 2 seconds. The host can retry after timeout.

## Managing modem commands

User protocol and commands can be added to the modem example.

- `at_defs.h` contains the definitions of the modem. Command format, error codes and system states are defined here
- `at.h` is the header file to the application
- `at.c` is the realization of the protocol

### Adding/removing a command

1. Add/remove a command label in the relevant query/control command enumeration in `at_defs.h`. It should be before the element ends with NUM.

2. Define/remove the command string in the relevant const array in `at_defs.h`. The order of the elements in the array must be the same as the correspondent enumeration.

3. Declare/remove the command handling function in `at.c`.

4. Add/remove the correspondence between the command label and handling function in the handler table in `at.c`.

### Adding/removing an error code

1. Add/remove an error code label in the error code enumeration in `at_defs.h`.

2. Define/remove the error code string In ErrorCodes[] array in `at_defs.h`. The order should be the same as the enumeration.

3. Use the `ATRespond` function to return an error code to the host.

## Modem protocol test

`at_test.sh` can be used to verify the AT command set of the modem example. Host simulator test and real hardware test can be issued.

### Dependencies

- Development environment setup following [developer.myriota.com](https://developer.myriota.com)
- Simulation library which can be downloaded by running the `./get_sim_lib.sh` command under the SDK root directory
- The testing tool `expect`: run `sudo apt install expect` if not installed

### Host simulator test

This mode can be used to quickly verify AT command protocol. "Timeout!" will be printed for each failure of a command test.

1. Build the modem example application in the host simulator mode.

    `make clean; MODULE=g2/sim make`

2. Run the test script.

    `mkdir -p obj; >obj/debug.log >obj/modem.log; ./at_test.sh "./at_modem 1>obj/debug.log" obj/modem.log`

Two log files will be generated in the `obj` folder.

### Real hardware test in Lab Mode

GNSS fix will be skipped in this mode.

1. Build the modem example application in Lab Mode.

    `make clean; LAB_TEST=1 make`

2. Program the board.

    `updater.py -u at_modem.bin -p <PORT> -s`

    `<PORT>` is the programming port. E.g. `updater.py -u at_modem.bin -p /dev/ttyUSB0 -s`

3. Run the test script and reset the board.

    `./at_test.sh <PORT>`

    `<PORT>` is the AT communication port. E.g. `./at_test.sh /dev/ttyUSB1`

## Modem hardware testing example

The application also shows how to run multiple hardware tests quickly without waiting for the modem to enter the READY state. By sending the string "test" to the modem 5 seconds after power-up, the application enters hardware test mode. The application waits for 3 seconds to detect the string before running into normal mode. The following items will be tested:

### GNSS

There are two options for this test:

1. GNSS hardware sanity test only

Default option. The test passes if the modem successfully receives a certain number of NMEA sentences from the GNSS hardware.

2. Full test

This can be done by setting `GNSS_COMM_TEST_ONLY` to `false` in `hardware_test.c`. The test passes if the modem can successfully acquire a GNSS fix.

### Radio TX

This test can be used to do TX power, TX current draw and TX/RX path connectivity checks. More information can be found in the [readme file](https://github.com/Myriota/SDK/blob/master/examples/rf_test/tx/README.md) of RF TX test example.

### Radio RX

This test can be used to check the RX path. More information can be found in the [readme file](https://github.com/Myriota/SDK/blob/master/examples/rf_test/rx/README.md) of RF RX test example.

Test results are output to both modem communication interface and serial debug interface.

## Disable module GNSS fix

It can be achieved by setting the `DISABLE_GNSS_FIX` macro to `1` in `main.c`. The GNSS fix process will be skipped and the time and location will be set to initial values. The host will need to feed the real time and location into the module using the time and location control commands.

## at_client.py

The `at_client.py` tool provides a simple host-side controller for testing and debugging AT Modem firmware. It supports both Myriota Ultralite Developer Toolkit and custom hardware, offering features such as message scheduling and raw AT-command interaction.

### Key Features

- **Cross-platform**: Supports Windows, Linux, and Pi.
- **Compatibility**: Supports `at_modem_v2.1.0` and later, including `skip_gnssfix` variant.
- **Simulated Scheduler**: Sends 3 tracker messages per day by default, each containing 1 location.
- **Raw AT Command Support**: Enables raw AT command interaction for debugging.

### Usage Examples

- Run tracker mode using the skip_gnss firmware, with port auto-detection enabled and a default rate of 3 messages per day.

    `python at_client.py --tracker --skip-gnss`

- Send 8 tracker messages using the gnssfix enabled firmware on Linux

    `python at_client.py --tracker 8 --port /dev/ttyUSB0`

- Run raw AT command mode on COM3 under Windows

    `python at_client.py --raw --port COM3`

### Message Format

| Field name        | Type     | Scale / Meaning                          | Example value               |
| ----------------- | -------- | ---------------------------------------- | --------------------------- |
| `sequence_number` | uint16_t | Sequence Number                          | `42`                        |
| `location_count`  | uint8_t  | Location number in the message           | `1`                        |
| `latitude`        | int32_t  | Scaled by **1e⁷** (degrees × 10,000,000) | `-891234567` → −89.1234567° |
| `longitude`       | int32_t  | Scaled by **1e⁷**                        | `1791234567` → 179.1234567° |
| `time`            | uint32_t | Unix epoch timestamp of last fix         | `1715584647`                |

### Message Decoding

The message can be decoded using the tracker example's [unpack.py](https://github.com/Myriota/SDK/blob/master/examples/tracker/unpack.py).

### Important
- **Firmware**: Use `at_modem_v2.1.0.bin` or later for the gnssfix enabled version or `at_modem_skip_gnssfix_v2.1.0.bin` or later with `-g 0` for skip-GNSS version.
- **Operation**: The host machine must remain active to avoid disrupting message scheduling.
