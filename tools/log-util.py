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


from __future__ import print_function
import sys
import struct
import time
import serial
import serial.tools.list_ports
import argparse
import binascii
import signal
import sys

errors = {
    0: "Internal test",
    1: "Factory reset",
    2: "Watchdog reset",
    3: "System states",
    4: "MCU faults",
    5: "Invalid key",
    6: "Application starts",
    7: "Assertion failure",
    8: "Memory error",
    9: "Stack low in space",
    10: "Stack overflow",
    11: "Module ID",
    12: "Reset reason",
    13: "Periodic states dump",  # New system states dump
    14: "Suspend mode enable",
    15: "Suspend mode disable",
    383: "Satellite communication stats",  # User 0xFF
}

unpack_strings = {
    "Internal test": "<II",
    "Watchdog reset": "<III",
    "System states": "<IIIIIIIIIIII",
    "MCU faults": "<IIII",
    "Application starts": "<QI",
    "Assertion failure": "<II",
    "Stack low in space": "<II",
    "Stack overflow": "<I",
    "Module ID": "<I",
    "Reset reason": "<I",
    "Periodic states dump": "<IIIIIIIIIII",
    "Satellite communication stats": "<IHHHH",
}

contents = {
    "Internal test": ["Test1", "Test2"],
    "Watchdog reset": ["Job ID", "PC", "LR"],
    "System states": [
        "Reset reason",
        "Wakeup times",
        "Last GNSS fix time",
        "Last job ID",
        "Watchdog timeouted job ID",
        "Watchdog timeout address",
        "Job ID of maximum run count",
        "Last log ID",
        "GNSS fix failures",
        "GNSS fix successes",
        "GNSS total fix time",
        "Job maximum run count",
    ],
    "MCU faults": ["CFSR", "LR", "PC", "PSR"],
    "Application starts": ["Build hash", "SDK version"],
    "Assertion failure": ["Return address", "Line number"],
    "Stack low in space": ["JobId", "StackUsage"],
    "Stack overflow": ["JobId"],
    "Module ID": ["Module ID"],
    "Reset reason": ["Reset reason"],
    "Periodic states dump": [
        "Reset reason",
        "Wakeup count",
        "Last GNSS fix time",
        "GNSS total fix time",
        "GNSS fix failures",
        "GNSS fix successes",
        "Last job ID",
        "Job ID of maximum run count",
        "Job maximum run count",
        "User jobs run count",
        "Last log ID",
    ],
    "Satellite communication stats": [
        "Timestamp",
        "RX attempt",
        "RX success",
        "RX unverified",
        "TX attempt",
    ],
}

reset_reasons = {
    0: "Power cycle",
    1: "Hardware nRST",
    2: "Hardware watchdog",
    3: "Software reset",
    4: "System lockup",
    5: "Brownout (battery low)",
    6: "Others",
    7: "Software watchdog",
    8: "MCU fault",
    9: "User requested reset via SDK API",
    10: "System sleep failure",
}

# if using python2, rename raw_input to input
try:
    input = raw_input
except NameError:
    pass


def get_ports():
    ports = set()
    for port in serial.tools.list_ports.comports():
        ports.add(port.device)
    return ports


def detect_port():
    print(
        "Please connect/reconnect the USB cable ",
        end="",
    )
    sys.stdout.flush()
    existing_ports = get_ports()
    while len(get_ports().difference(existing_ports)) == 0:
        existing_ports = get_ports()
        time.sleep(0.5)
        print(".", end="")
        sys.stdout.flush()
    print("")
    return get_ports().difference(existing_ports).pop()


def dump_bytes(bytes):
    for b in bytes:
        print("%02x" % b, end=" ")
    print("")


# Read number of bytes rounded to 4, and dump if readback is too short
def read_and_check_completion(file, length):
    # Round to 4 bytes boundary
    bytes = bytearray(file.read(int((length + 3) // 4 * 4)))
    if len(bytes) < length:
        print("Incomplete log payload")
        dump_bytes(bytes)
        return None
    else:
        return bytes


def decode_log(logfile):
    with open(logfile, "rb") as binary_file:
        # Entry format, in little endian
        # |0-1|2-3|4-?|
        # |Length|Code|Payload|
        is_empty = True
        while True:
            bytes = binary_file.read(8)
            if len(bytes) == 0:
                is_empty = False
                return is_empty
            if len(bytes) < 8:
                if not is_empty:
                    print("Incomplete log entry")
                binary_file.close()
                return is_empty
            timestamp = struct.unpack("<IHH", bytes)
            timestamp, length, code = struct.unpack("<IHH", bytes)
            # End of the log
            if timestamp == 0xFFFFFFFF:
                return is_empty
            is_empty = False
            # UTC time
            time_string = time.strftime(
                "%Y-%m-%d %H:%M:%S UTC", time.gmtime(float(timestamp))
            )
            try:
                key = errors[code]
                if length == 0:
                    print("====%s %s====" % (time_string, key))
                    continue
                else:
                    bytes = read_and_check_completion(binary_file, length)
                    if bytes is None:
                        print("====%s %s====" % (time_string, key))
                        return is_empty
                    else:
                        values = struct.unpack(unpack_strings[key], bytes)
                        print("====%s %s====" % (time_string, key))
                        for k, v in zip(contents[key], values):
                            print("%s : 0x%08x(%d)" % (k, v, v))
                        if key == "Reset reason":
                            print("%s" % reset_reasons[values[0]])
                        continue
            except struct.error:
                if code < 0x80:
                    print("====%s %s====" % (time_string, key))
                    print("Unable to decode")
                    dump_bytes(bytes)
                    continue
                else:
                    binary_file.seek(-len(bytes), 1)
            except KeyError:
                pass
            if code >= 0x80 and code <= 0x80 + 0xFF:
                print("====%s User error code %d====" % (time_string, (code - 0x80)))
                if length != 0:
                    bytes = read_and_check_completion(binary_file, length)
                    if bytes is None:
                        return is_empty
                    else:
                        dump_bytes(bytes)
                        continue
            print("====%s Unknown error code %d====" % (time_string, code))
            if length != 0:
                bytes = read_and_check_completion(binary_file, length)
                if bytes is None:
                    return is_empty
                else:
                    dump_bytes(bytes)


def capture_bootloader(portname, baudrate, wait_flag):
    if wait_flag:
        print("Waiting for serial port", portname)
        while True:
            try:
                ser = serial.Serial(
                    port=portname,
                    baudrate=baudrate,
                    parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_ONE,
                    bytesize=serial.EIGHTBITS,
                    xonxoff=0,
                    rtscts=0,
                    timeout=0.5,
                )
                break
            except (OSError, serial.SerialException):
                time.sleep(0.5)
                pass
    else:
        try:
            ser = serial.Serial(
                port=portname,
                baudrate=baudrate,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                xonxoff=0,
                rtscts=0,
                timeout=0.5,
            )
        except (OSError, serial.SerialException):
            print("Failed to open", portname)
            sys.exit(1)

    ser.reset_input_buffer()
    ser.write(b"U")
    ser.flush()
    out = ser.readline()
    if out is not None:
        out += ser.readline()
        out += ser.readline()
    if b"Unknown" in out or b"Bootloader" in out:
        return ser
    print("Please reset the device", end="")
    sys.stdout.flush()

    # Try to capture the bootloader
    while True:
        ser.write(b"U")
        ser.flush()
        out = ser.readline()
        if b"Bootloader" in out or b"Unknown" in out:
            print("")
            break
        else:
            time.sleep(0.5)
            print(".", end="")
            sys.stdout.flush()

    ser.reset_input_buffer()
    return ser


def read_log(ser):
    print("Start reading the log")
    # Start dumping
    ser.write(b"x")
    dump = b""
    while True:
        out = ser.readline()
        if len(out) > 16:
            dump += out.strip()
        elif len(out) == 0:
            break
    return dump


def purge_log(ser):
    # Start purging
    while True:
        ser.write(b"!")
        out = ""
        out = ser.readline()
        if b"purged" in out:
            break
    print("The log has been purged")


def signal_handler(signal, frame):
    if serial_port is not None:
        serial_port.close()
    sys.exit(0)


serial_port = None


def main():
    global serial_port

    signal.signal(signal.SIGINT, signal_handler)
    try:
        signal.signal(signal.SIGPIPE, signal_handler)
    except AttributeError:
        pass

    infile = ""
    outfile = ""
    port_name = "None"

    parser = argparse.ArgumentParser(
        description="Myriota device log decoder",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-i", "--ifile", dest="infile", help="decode local log FILE", metavar="FILE"
    )
    parser.add_argument(
        "-o",
        "--ofile",
        dest="outfile",
        default="log.bin",
        metavar="FILE",
        help="write log to FILE",
    )
    parser.add_argument(
        "-p",
        "--port",
        dest="portname",
        metavar="PORT",
        default=port_name,
        help="serial PORT the Myriota device is connected to, e.g. /dev/ttyUSB0",
    )
    parser.add_argument(
        "-w",
        "--wait",
        dest="wait_flag",
        action="store_true",
        default=False,
        help="wait for PORT to be available",
    )
    parser.add_argument(
        "-x",
        "--purge",
        dest="purge_flag",
        action="store_true",
        default=False,
        help="purge the log",
    )
    parser.add_argument(
        "-b",
        "--baudrate",
        dest="baud_rate",
        metavar="BAUDRATE",
        default=115200,
        help="set the serial port BAUDRATE",
    )

    parser.add_argument(
        "-d",
        "--default-port",
        dest="default_port_flag",
        action="store_true",
        default=False,
        help="use default serial port",
    )

    args = parser.parse_args()

    port_name = args.portname
    if args.default_port_flag:
        if serial.tools.list_ports.comports():
            port_name = serial.tools.list_ports.comports()[0].device

    if port_name == "None" and args.infile is None and args.purge_flag is False:
        port_name = detect_port()
    if args.infile:
        infile = args.infile
    else:
        if args.purge_flag:
            if port_name == "None":
                port_name = detect_port()
        if args.outfile:
            if port_name == "None":
                port_name = detect_port()
            outfile = args.outfile

    if args.purge_flag:
        if serial_port is None:
            serial_port = capture_bootloader(port_name, args.baud_rate, args.wait_flag)
        answer = ""
        while answer not in ["y", "n"]:
            answer = input("Do you want to purge the log [y/n]? ").lower()
            if answer == "y":
                print("Using serial port", port_name, args.baud_rate)
                purge_log(serial_port)
        sys.exit(0)

    if not infile:
        if port_name != "None":
            print("Using serial port", port_name, args.baud_rate)
            serial_port = capture_bootloader(port_name, args.baud_rate, args.wait_flag)
            dump = read_log(serial_port)
            print("Writing log to", outfile)
            with open(outfile, "wb") as binary_file:
                binary_file.write(binascii.unhexlify(dump))
                binary_file.close()
            infile = outfile
    print("Decoding", infile)
    if decode_log(infile):
        print("No log found")

    if serial_port is not None:
        serial_port.close()


if __name__ == "__main__":
    main()
