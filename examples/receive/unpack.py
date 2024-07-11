#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2023, Myriota Pty Ltd, All Rights Reserved
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


# Unpacker for the receive example
# Usage:
# unpack.py -x 6e07b66401009b3cb56448656c6c6f2ccccccccc
# or
# echo 6e07b66401009b3cb56448656c6c6f2ccccccccc | unpack.py


import argparse
import struct
import json
import fileinput


def unpack(packet):
    timestamp, count_rx, timestamp_rx = struct.unpack(
        "<IHI", bytearray.fromhex(packet[0:20])
    )

    return [
        {
            "Timestamp": timestamp,
            "CountReceive": count_rx,
            "TimestampReceive": timestamp_rx if count_rx > 0 else None,
            "MessageReceive": str(packet[20:40]) if count_rx > 0 else None,
        }
    ]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Unpack hexadecimal data from receive example.",
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
