#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2016-2025, Myriota Pty Ltd, All Rights Reserved
# SPDX-License-Identifier: BSD-3-Clause-Attribution
#
# This file is licensed under the BSD with attribution  (the "License"); you
# may not use these files except in compliance with the License.
#
# You may obtain a copy of the License here:
# LICENSE-BSD-3-Clause-Attribution.txt and at
# https://spdx.org/licenses/BSD-3-Clause-Attribution.html
#
# See the License for the specific language governing permissions and
# limitations under the License.


# Unpacker for the tracker example.
# Usage:
# unpack.py -x 01000130de2eebb0239d525f827266cccccccccc
# or
# echo "01000130de2eebb0239d525f827266cccccccccc" | unpack.py

import argparse
import struct
import json
import fileinput


def unpack(packet):
    packet_byte = bytearray.fromhex(packet)
    locations = []
    offset = 0

    # Unpack the sequence number (2 bytes)
    sequence_number = struct.unpack("<H", packet_byte[offset : offset + 2])[0]
    offset += 2

    # Unpack the number of locations (1 byte)
    location_count = struct.unpack("<B", packet_byte[offset : offset + 1])[0]
    offset += 1

    location_sz = 12  # latitude (4 bytes), longitude (4 bytes), timestamp (4 bytes)
    # Unpack the location array
    for _ in range(location_count):
        # Unpack single location
        lat, lon, timestamp = struct.unpack(
            "<iiI", packet_byte[offset : offset + location_sz]
        )
        offset += location_sz
        locations.append(
            {
                "Latitude": lat / 1e7,
                "Longitude": lon / 1e7,
                "Timestamp": timestamp,
            }
        )

    return [
        {
            "Sequence number": sequence_number,
            "Location count": location_count,
            "Locations": locations,
        }
    ]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Unpack hexadecimal data from tracker example.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-x", "--hex", type=str, default="-", help="Packet data in hexadecimal format"
    )
    args = parser.parse_args()

    d = []
    if args.hex == "-":
        for line in fileinput.input():
            d = d + unpack(line.strip())
    else:
        d = d + unpack(args.hex)

    print(json.dumps(d))
