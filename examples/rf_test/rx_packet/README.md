# Satellite radio downlink packet verification test

This is an example code to show case how to verify downlink packet reception using Myriota Module API:
* RxStatsGet()

`RxStatsGet()` output 3 statistics:
* attempts: Indicates how many times the module has attempted reception
* verified: Indicates how many received packets has been correctly verified
* unverified: Indicates how many received packets that can not be verified.

Downlink packet verification can be done in the Lab or as part of deployment.
* Lab test uses `LabRx` network information
* Field test uses the default network information

## Lab Test

#### Downlink Packet Transmitter:
* Myriota Service 2 development board (M2-2x) or Sense&Locate board (SL2-1) can be used to transmit downlink packet.
* Pre-built binary `downlink_packet_tx.bin` needs to be programmed to transmit downlink packets from the device
* Program using the following command:
```bash
updater.py -t downlink_packet_tx.bin
```

#### Build
* Build for testing in the Lab, with test network information. The default poll interval is 10 seconds
```bash
cd examples/rf_test/rx_packet
SATELLITES=LabRx make clean rx_packet
```

#### Device Under Test
* Program Device Under Test (DUT) with the following command
```bash
updater.py -f system.img -u rx_packet.bin -s -l
```

LED is used to indicate receiver status for each poll:

| Status                                  | LED pattern       |
|-----------------------------------------|-------------------|
| at least one good packet is received    | blink three times |
| received a packet, but unable to verify | blink twice       |
| no packet is received                   | blink once        |

* Example of DUT serial debug output:
```
  Starting application

  1483228839 rx packets: Attempts 1      Unverified 0      Verified 1

  1483228855 rx packets: Attempts 3      Unverified 0      Verified 3

  1483228872 rx packets: Attempts 5      Unverified 0      Verified 5
````

- Pressing GPIO wakeup button will reset the DUT's receiver and statistics


## Field Test

#### Build
* Build for field testing
* Default poll interval is 2 hours
* Results are logged to flash partition

```bash
cd examples/rf_test/rx_packet
make clean rx_packet
```

#### DUT
* The DUT should be deployed with a clear sky view
* At least one good downlink satellite pass is required to get the statistics
* Depending on the configured polling interval, the statistics would be queried and written to the flash
* To view the test result, connect the device to a USB port, and press the wakeup button. Example of output is shown below:
```
  1629153911 rx packets: Attempts 52    Unverified 1      Verified 33
```
* LED patterns for the field test is same as mentioned above
* Statistics would be reset to zero after pressing the wakeup button

* To post-process the results, download the log first by `log-util.py` and then use the `rx_packet_unpack.py` script to get the human-readable results using a command like `log-util.py -i log.bin -p /dev/ttyUSB0 | grep -A1 "User error code 1" | grep -v "User error code 1" | tr -d ' ' | ./rx_packet_unpack.py`
