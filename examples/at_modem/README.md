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

- Modem sends `+STATE=INITIALIZING` when system is initializing
- Modem sends `+STATE=GNSS_ACQ` if system failed to get initial GNSS fix acquisition and starts continual retrying automatically
- Modem sends `+STATE=READY` if system has acquired GNSS fix
- The state changes to READY once GNSS fix has acquired which can be queried by `STATE` command
- Only in READY state can a scheduled message be transmitted

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
| Message queue     | MSGQ     | Number of free bytes in the message queue | - |
| System state      | STATE    | INITIALIZING, GNSS_ACQ or READY | Refer to power up logic |
| SDK version       | VSDK     | SDK version | Format: major.minor.patch |
| Module ID         | MID      | Module ID and part number | E.g. 0012345678 M1-24 |
| Registration code | REGCODE  | Registration code | - |
| Get time          | TIME     | Unix epoch time | E.g. 1595911282 |
| Get location      | LOCATION | Latitude and longitude of last GNSS fix, scaled by 1e7 | E.g. -349205499,1386086737 |
| Get suspend mode  | SUSPEND  | 0: disabled, 1:enabled | - |

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
| Schedule message      | SMSG     | Hex string of the message | The length should be even and no more than 40 hex characters |
| Change suspend mode   | SUSPEND  | 1 to enable and 0 to disable | - |

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
| MESSAGE_TOO_LONG   | Scheduled message is over 20 bytes | Reduce message size |
| BUFFER_OVERFLOW    | Modem RX buffer overflow | Reduce UART frame length |
| UNKNOWN_QUERY_CMD | Query identifier "=?" detected but no command match | Check query command list |
| UNKNOWN_CONTROL_CMD | Control format matched but command is not found | Check control command list |
| INVALID_COMMAND    | Command format error, can be cause by prefix/terminator match or command/parameter too long | Check command format or monitor debug port for detail reason |

### Timeout and retry

The timeout time of modem responses is 2 seconds. The host can retry after timeout.

## Managing modem commands

User protocol and commands can be added to the modem example.

- `at_defs.h` contains the definitions of the modem. Command format, error codes and system states are defined here
- `at.h` is the header file to the application
- `at.c` is the realization of the protocol

### Adding/removing a command

1. Add/remove a command label in the relevant query/control command enumeration in `at_defs.h`. It should be before the element ends with NUM.

2. Define/remove the command string in the relevant const array in `at_defs.h`. The order of the elements in the array must be the same with the correspondent enumeration.

3. Declare/remove the command handling function in `at.c`.

4. Add/remove the correspondence between the command label and handling function in the handler table in `at.c`.

### Adding/removing an error code

1. Add/remove an error code label in the error code enumeration in `at_defs.h`.

2. Define/remove the error code string In ErrorCodes[] array in `at_defs.h`. The order should be the same with the enumeration.

3. Use ATRespond function to return error code to host.

## Modem command test

`at_test.sh` can be used to verify the AT command set of the modem example. Host simulator test and real hardware test can be issued.

### Dependencies

- Development environment setup following [developer.myriota.com](https://developer.myriota.com)
- Simulation library which can be downloaded by running `./get_sim_lib.sh` command under SDK root directory
- The testing tool `expect`: run `sudo apt install expect` if not installed

### Host simulator test

This mode can be used to quickly verify AT command protocol. "Timeout!" will be printed for each failure of command test.

1. Build the modem example application in host simulator mode.

`make clean; MODULE=g2/sim make`

2. Run the test script.

`mkdir -p obj; >obj/debug.log >obj/modem.log; ./at_test.sh "./at_modem 1>obj/debug.log" obj/modem.log`

Two log files will be generated in `obj` folder.

### Real hardware test in Lab Mode

GNSS fix will be skipped in this mode.

1. Build the modem example application in Lab Mode.

`make clean; SATELLITES=Lab make`

2. Program the board.

`updater.py -u at_modem.bin -p <PORT> -s`

`<PORT>` is the programming port. E.g. `updater.py -u at_modem.bin -p /dev/ttyUSB0 -s`

3. Run the test script and reset the board.

`./at_test.sh <PORT>`

`<PORT>` is the AT communication port. E.g. `./at_test.sh /dev/ttyUSB1`
