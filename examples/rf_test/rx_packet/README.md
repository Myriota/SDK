# Satellite radio downlink packet verification test

This example showcases how to verify downlink packet reception using the Myriota Module API `RxStatsGet()`.

The API `RxStatsGet()` outputs 2 statistics:
* attempts: Indicates how many times the module has attempted reception
* successes: Indicates how many received packets have been received correctly

## Build and program
* Build the example app for for field testing with
    ```bash
    cd examples/rf_test/rx_packet
    make clean rx_packet
    ```
* The default polling interval is 2 hours
* Results are logged to flash
* Program the Device Under Test (DUT) with the following command
    ```bash
    updater.py -u rx_packet.bin -s -l
    ```

## Field test

* The DUT should be deployed with a clear sky view
* Once the DUT has joined the network, at least one good downlink satellite pass is required to get the statistics
* The statistics are queried and written to flash with the configured polling interval
* To view the test result, connect the device to a USB port, and press the wakeup button. Example of output is shown below:
    ```
    1715584647 rx packets: Attempts 52  Successes 33
    ```
* LED patterns are used to indicate receiver status for each poll
    * blink twice - at least one good packet is received
    * blink once  - no packet is received
* Statistics are reset to zero after pressing the wakeup button
* To post-process the results, download the log first with `log-util.py` and then use the `rx_packet_unpack.py` script to get the human-readable results using a command like
    ```
    log-util.py -i log.bin -p /dev/ttyUSB0 \
      | grep -A1 "User error code 1" | grep -v "User error code 1" | tr -d ' ' \
      | ./rx_packet_unpack.py
    ```
