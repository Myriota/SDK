#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2021, Myriota Pty Ltd, All Rights Reserved
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


# Unpacker for the snl example.
# Usage:
# unpack.py -x 1000593033eb7e02a652b47c746054100000cccc
# or
# echo "1000593033eb7e02a652b47c746054100000cccc" | unpack.py

import argparse
import struct
import json
import fileinput


def unpack(packet):
    num, lat, lon, timestamp, current = struct.unpack(
        "<HiiII", bytearray.fromhex(packet[0:36])
    )
    return [
        {
            "Sequence number": num,
            "Latitude": lat / 1e7,
            "Longitude": lon / 1e7,
            "Timestamp": timestamp,
            "Current": current,
        }
    ]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Unpack hexadecimal data from snl example.",
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
