#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2021-2024, Myriota Pty Ltd, All Rights Reserved
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


# Unpacker for the downlink receiver statistics
# Usage:
# ./rx_packet_unpack.py -x 330000002b000000
# echo "330000002b000000" | ./rx_packet_unpack.py
#
# Usage from log-util.py "User error code 1" output
# echo "33 00 00 00 2b 00 00 00" | tr -d ' ' | ./rx_packet_unpack.py

import argparse
import struct
import json
import fileinput


def unpack(packet):
    attempts, successes = struct.unpack("<HH", bytearray.fromhex(packet[0:8]))
    return [
        {
            "attempts": attempts,
            "successes": successes,
        }
    ]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Unpack hexadecimal data from rx_packet example",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-x", "--hex", type=str, default="-", help="RxStats in hexadecimal format"
    )
    args = parser.parse_args()

    try:
        if args.hex == "-":
            for line in fileinput.input():
                try:
                    print(json.dumps(unpack(line.strip())))
                except Exception as e:
                    print(line.strip(), e)
        else:
            print(json.dumps(unpack(args.hex)))
    except KeyboardInterrupt as e:
        exit(e)
    except Exception as e:
        exit(e)
