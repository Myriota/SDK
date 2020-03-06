#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2016-2020, Myriota Pty Ltd, All Rights Reserved
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
# unpack.py -x 0000aea02feb06fe9d52b89f3e5ccccccccccccccc
# or
# echo "0000aea02feb06fe9d52b89f3e5ccccccccccccccc" | unpack.py

import argparse
import struct
import json
import fileinput

def unpack(packet):
    num, lat, lon, timestamp = struct.unpack("<HiiI", bytearray.fromhex(packet[0:28]))
    return [{'Sequence number': num, 'Latitude': lat/1e7, 'Longitude': lon/1e7, 'Timestamp': timestamp}]

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Unpack hexadecimal data from tracker example.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-x', '--hex', type=str, default="-",
                        help='Packet data in hexadecimal format')
    args = parser.parse_args()

    d = []
    if args.hex == "-":
        for line in fileinput.input():
            d = d + unpack(line.strip())
    else:
        d = d + unpack(args.hex)

    print(json.dumps(d))
