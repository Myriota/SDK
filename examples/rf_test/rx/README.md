# Satellite radio RX test

## RSSI readings

RSSI readings are directly from the radio chip receiver which does not account for RF front end.
Compensation is needed to match the output from the equipment like signal generator.
Contact support@myriota.com to get further information.

## Noise measurement

The RF rx test example can used to measure noise which may affect the downlink performance.

External power supply or USB to serial cable may introduce noise. To make the measurement results accurate and representative, logging the RSSI readings to flash instead of getting the debug output may be necessary.
Flash logging can be enabled by uncommenting the line `LogAdd(0, &RSSI, sizeof(RSSI));`.
To post process the results, download the log first by `log-util.py` and then use a command like
`log-util.py -i log.bin -p /dev/ttyUSB0 | grep ff | cut -d' ' -f1 | tr '[:lower:]' '[:upper:]' | xargs -I % sh -c 'echo "ibase=16; % -100" | bc'` to get the human-readable results.

To measure board noise, it is recommended to put the device in the enclosure and have the antenna connected.
The measurement should be done in a radio silent environment, ideally inside a faraday cage.

To measure environment noise, the device can be deployed in the actual environment for the deployment.
