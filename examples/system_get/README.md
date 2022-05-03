# System information retrieval

The user application can retrieve system information from the module via system update APIs.
And the information can be uploaded to [Myriota cloud endpoints](upload_api.yaml) for further processing.

## Network information
The network information can be uploaded to the API endpoint `/network-info` together with the module ID.

## Diagnostics
The diagnostics data consists of 8 bytes of Unix epoch timestamp in little-endian and array of diagnostics packets with the length of 29 bytes. Multiple entries of timestamp and diagnostics packet pairs can be uploaded to the API endpoint `/packets`.
This is the example data unpacker in Python:
```python
import argparse
import json
import struct
def split_binary(input_file):
    resulting_messages = []
    with open(input_file, "rb") as input_bytes:
        data = input_bytes.read()
        timestamp_bytes = data[0:8]
        # Unpack timestamp which is encoded as a "long int" (8 bytes)
        timestamp = struct.unpack("l", timestamp_bytes)[0]
        for offset in range(8, len(data), 29):
            message_hex = data[offset : offset + 29].hex()
            message = {"timestamp": timestamp, "message": message_hex}
            resulting_messages.append(message)
    print(json.dumps(resulting_messages))
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i", "--input-file", help="Diagnostics data file to split into separate packets"
    )
    args = parser.parse_args()
    split_binary(args.input_file)
```
