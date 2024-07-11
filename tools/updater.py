#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2016-2021, Myriota Pty Ltd, All Rights Reserved
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
import time
import serial
import serial.tools.list_ports
import argparse
import os
import struct
import tempfile
from io import BytesIO
import signal
import sys


version = "1.2"


def reset_device():
    port_number = 0
    retry = 6
    try:
        import ftd2xx as ftd

        while True:
            try:
                d = ftd.open(port_number)
                if d.getDeviceInfo()["description"] == b"G2":
                    break
                d.close()
                port_number += 1
            except ftd.DeviceError:
                if port_number == 0 and retry >= 0:
                    time.sleep(0.5)
                    retry -= 1
                    continue
                print("Please reset the device", end="")
                return

        print("Resetting Myriota device", end="")
        d.setBitMode(0xF0, 0x20)  # set reset pin low
        time.sleep(0.1)
        d.setBitMode(0xF1, 0x20)  # set reset pin high
        time.sleep(0.1)
        d.setBitMode(0, 0x20)
        d.close()
    except (OSError, ImportError) as e:
        print("Please reset the device", end="")
        return


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


def open_serial_port(portname, baudrate):
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
        sys.stderr.write("Failed to open %s\n" % portname)
        sys.exit(1)
    return ser


def wait_for_serial_port(portname, baudrate):
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
            return ser
        except (OSError, serial.SerialException):
            time.sleep(0.5)
            pass


def capture_bootloader(ser, port_name, br):
    # return straightaway if already in bootloader
    ser.reset_input_buffer()
    ser.write(b"U")
    ser.flush()
    out = ser.readline()
    if out is not None:
        out += ser.readline()
        out += ser.readline()
    if b"Unknown" in out or b"Bootloader" in out:
        ser.readline()
        ser.readline()
        return ser
    ser.close()

    # reset Myriota device
    reset_device()

    ser = open_serial_port(port_name, br)
    sys.stdout.flush()
    ser.reset_input_buffer()

    # Try to capture the bootloader
    MAX_RETRIES = 10
    retries = 0
    while True:
        ser.write(b"U")
        ser.flush()
        line = ser.readline()
        out = line
        if b"Bootloader" in line:
            print("\n")
            ser.readline()
            ser.readline()
            ser.readline()
            ser.readline()
            break
        if line is not None:
            out += ser.readline()
            if b"Unknown" in out:
                break
        retries += 1
        if retries > MAX_RETRIES:
            sys.stderr.write("failed to connect to the board\n")
            sys.exit(1)
        time.sleep(0.5)
        print(".", end="")
        sys.stdout.flush()

    ser.reset_input_buffer()
    return ser


def _bytecrc(crc, poly, n):
    mask = 1 << (n - 1)
    for i in range(8):
        if crc & mask:
            crc = (crc << 1) ^ poly
        else:
            crc = crc << 1
    mask = (1 << n) - 1
    crc = crc & mask
    return crc


def calc_crc(data):
    poly = 0x11021
    bit_size = 16
    table = [_bytecrc(i << (bit_size - 8), poly, bit_size) for i in range(256)]
    crc = 0
    for b in data:
        crc = table[b ^ ((crc >> 8) & 0xFF)] ^ ((crc << 8) & 0xFF00)
    return crc


def xmodem_send(serial, file, ncg, quiet=True):
    SOH = bytes(bytearray([0x01]))
    EOT = bytes(bytearray([0x04]))
    ACK = bytes(bytearray([0x06]))
    NAK = bytes(bytearray([0x15]))
    NCG = b"C"
    DATA_SIZE = 128
    if not ncg:
        t = 0
        while True:
            if serial.read(1) != NCG:
                t = t + 1
                if t == 10:
                    print("*", end="")
                    sys.stdout.flush()
                    return False
            else:
                break
    pn = 1
    file.seek(0)
    data = bytearray(file.read(DATA_SIZE))
    tx_size = 0
    while data:
        for i in range(DATA_SIZE - len(data)):
            data += bytes(bytearray([0xFF]))
        crc = calc_crc(data)
        MAX_RETRIES = 1
        retries = 0
        while True:
            serial.write(SOH)
            serial.write(bytes(bytearray([pn])))
            serial.write(bytes(bytearray([0xFF - pn])))
            serial.write(data)
            serial.write(bytes(bytearray([(crc & 0xFF00) >> 8])))
            serial.write(bytes(bytearray([crc & 0xFF])))
            serial.flush()
            answer = serial.read(1)
            if answer == NAK:
                if not quiet:
                    print("!", end="")
                    sys.stdout.flush()
                retries += 1
                if retries > MAX_RETRIES:
                    return False
                else:
                    continue
            if answer == ACK:
                retries = 0
                tx_size += DATA_SIZE
                if not tx_size % (1024 * 10):
                    print(".", end="")
                    sys.stdout.flush()
                break
            # If got nothing, exit
            serial.write(EOT)
            serial.flush()
            if not quiet:
                print("$", end="")
                sys.stdout.flush()
            return False
        data = bytearray(file.read(DATA_SIZE))
        pn = (pn + 1) % 256
    serial.write(EOT)
    serial.flush()
    answer = serial.read(1)
    if answer == NAK:
        return False
    return True


def update_stream(ser, command, stream):
    sys.stdout.flush()
    retries = 3
    update_in_progress = False
    while True:
        stream.seek(0)
        ser.reset_input_buffer()
        ser.write(bytearray(command.encode("utf-8")))
        ser.flush()
        out = b""
        out += ser.readline()
        out += ser.readline()
        out += ser.readline()
        out += ser.readline()
        ncg = False
        if b"C" in out:
            ncg = True
        else:
            if not update_in_progress and b"Fail" in out and retries <= 1:
                sys.stderr.write("partition error\n")
                sys.stderr.write(
                    "Please confirm if correct system image has been programmed\n"
                )
                return False
        if b"Ready" in out and not b"Fail" in out:
            update_in_progress = True
            if xmodem_send(ser, stream, ncg):
                stream.close()
                return True
        retries -= 1
        if retries == 0:
            break
        else:
            time.sleep(1)

    sys.stderr.write("failed\n")
    return False


def update_image(ser, command, filename, handle=None):
    try:
        if handle is None:
            stream = open(filename, "rb")
        else:
            stream = handle
    except IOError:
        sys.stdout.flush()
        sys.stderr.write("\nCan't open %s\n" % filename)
        return False

    stream.seek(0, os.SEEK_END)
    file_size = stream.tell()
    stream.seek(0, os.SEEK_SET)
    print(
        "\nProgramming %s (%dK) " % (filename, (file_size + 1023) / 1024),
        end="",
    )
    return update_stream(ser, command, stream)


def jump_to_app(ser):
    ser.write(b"b")
    ser.flush()


def get_id(ser):
    MAX_RETRIES = 2
    retries = 0
    while True:
        ser.write(b"i")
        ser.flush()
        out = b""
        ser.readline()
        out += ser.readline()

        if b"Unknown" in out:
            retries += 1
        else:
            print("ID:", end="")
            print(out.decode("utf-8"))
            break
        if retries > MAX_RETRIES:
            sys.stderr.write("Failed to read ID\n")
            break


def get_regcode(ser):
    MAX_RETRIES = 2
    retries = 0
    while True:
        ser.write(b"g")
        ser.flush()
        out = b""
        ser.readline()
        out += ser.readline()

        if b"Unknown" in out:
            retries += 1
        else:
            print("Registration code:", end="")
            print(out.decode("utf-8"))
            break
        if retries > MAX_RETRIES:
            sys.stderr.write("Failed to read regcode\n")
            break


def get_version(ser):
    MAX_RETRIES = 2
    retries = 0
    while True:
        ser.write(b"V")
        ser.flush()
        out = b""
        ser.readline()
        out += ser.readline()

        if b"Unknown" in out:
            retries += 1
        else:
            print(out.decode("utf-8"))
            break
        if retries > MAX_RETRIES:
            print(out.decode("utf-8"))
            break


def dump_debug_output(ser):
    print("Dumping debug output\n")
    while True:
        out = ser.readline()
        if len(out) != 0:
            print(out.decode("utf-8"), end="")
            sys.stdout.flush()


def signal_handler(signal, frame):
    if serial_port is not None:
        serial_port.close()
    sys.exit(0)


file_types = {
    1: "system image",
    2: "user application",
    3: "network information",
    4: "system image part 2",
}


serial_port = None
FIRMWARE_START_ADDRESS = 0x4000
header_length = 16
header_version = 0


def is_merged_binary(filename):
    try:
        with open(filename, "rb") as input_file:
            totalsize = os.stat(filename).st_size
            if totalsize <= header_length:
                return False

            offset = 0
            while offset < totalsize:
                input_file.seek(2, os.SEEK_CUR)
                ftype = struct.unpack("<H", input_file.read(2))[0]
                if not ftype in file_types:
                    return False

                flen = struct.unpack("<I", input_file.read(4))[0]
                reserved = struct.unpack("<I", input_file.read(4))[0]
                reserved = struct.unpack("<H", input_file.read(2))[0]
                checksum = struct.unpack("<H", input_file.read(2))[0]
                input_file.seek(0 - header_length, os.SEEK_CUR)
                data = bytearray(input_file.read(header_length + flen))
                data[14] = data[15] = 0
                if checksum != (calc_crc(data) & 0xFFFF):
                    return False

                offset += header_length + flen
            return True
    except IOError:
        sys.stderr.write("Can't open %s\n" % filename)
        sys.exit(1)


def append_merged_files(filename, command):
    with open(filename, "rb") as input_file:
        offset = 0
        while offset < os.stat(filename).st_size:
            header_version = struct.unpack("<B", input_file.read(1))[0]
            reserved = struct.unpack("<B", input_file.read(1))[0]
            ftype = struct.unpack("<H", input_file.read(2))[0]
            flen = struct.unpack("<I", input_file.read(4))[0]
            input_file.seek(8, os.SEEK_CUR)
            output_temp = tempfile.NamedTemporaryFile(mode="w+b", delete=False)
            output_temp.write(input_file.read(flen))
            if ftype == 1:
                command.append(
                    [
                        "a%x" % FIRMWARE_START_ADDRESS,
                        filename + "(" + file_types.get(ftype) + ")",
                        output_temp,
                    ]
                )
            if ftype == 2:
                command.append(
                    [
                        "s",
                        filename + "(" + file_types.get(ftype) + ")",
                        output_temp,
                    ]
                )
            if ftype == 3:
                command.append(
                    [
                        "o",
                        filename + "(" + file_types.get(ftype) + ")",
                        output_temp,
                    ]
                )
            if ftype == 4:
                command.append(
                    [
                        "S",
                        filename + "(" + file_types.get(ftype) + ")",
                        output_temp,
                    ]
                )
            output_temp.flush()
            offset += flen + header_length


def main():
    global serial_port

    signal.signal(signal.SIGINT, signal_handler)
    try:
        signal.signal(signal.SIGPIPE, signal_handler)
    except AttributeError:
        pass
    port_name = "None"

    parser = argparse.ArgumentParser(
        description="Myriota device updater " + version,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-f",
        "--system_image",
        dest="system_image_name",
        help="system image FILE to update with",
        metavar="FILE",
    )
    parser.add_argument(
        "-u",
        "--user_app",
        dest="user_app_name",
        help="user application or user application merged with network information FILE to update with",
        metavar="FILE",
    )
    parser.add_argument(
        "-n",
        "--network_info",
        dest="network_info_bin_name",
        help="network info binary FILE to update with",
        metavar="FILE",
    )
    parser.add_argument(
        "-m",
        "--merged_file",
        dest="merged_bin_name",
        help="merged FILE to update with",
        metavar="FILE",
    )
    parser.add_argument(
        "-r",
        "--raw",
        nargs=2,
        dest="raw_commands",
        action="append",
        metavar=("raw_command", "FILE"),
        help="use raw command and FILE pair to update, can have multiple",
    )
    parser.add_argument(
        "-t",
        "--test_image",
        dest="test_image_name",
        help="test image FILE to update with",
        metavar="FILE",
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
        "-s",
        "--start",
        dest="start_flag",
        action="store_true",
        default=False,
        help="start the application after update",
    )
    parser.add_argument(
        "-i",
        "--id",
        dest="get_id_flag",
        action="store_true",
        default=False,
        help="get ID",
    )
    parser.add_argument(
        "-c",
        "--regcode",
        dest="get_regcode_flag",
        action="store_true",
        default=False,
        help="get registration code",
    )
    parser.add_argument(
        "-v",
        "--version",
        dest="get_version_flag",
        action="store_true",
        default=False,
        help="get bootloader version (only support 0.9.0 or later)",
    )
    parser.add_argument(
        "-b",
        "--baudrate",
        dest="baud_rate",
        metavar="BAUDRATE",
        default=115200,
        help="set the serial port BAUDRATE between 9600 and 921600",
    )

    parser.add_argument(
        "-d",
        "--default-port",
        dest="default_port_flag",
        action="store_true",
        default=False,
        help="use default serial port",
    )

    parser.add_argument(
        "-l",
        "--listen-port",
        dest="listen_port_flag",
        action="store_true",
        default=False,
        help="listen to serial port",
    )

    parser.add_argument(
        "-x",
        "--debug",
        dest="debug",
        action="store_true",
        default=False,
        help="interactive debug mode",
    )

    args = parser.parse_args()

    if args.default_port_flag:
        if serial.tools.list_ports.comports():
            port_name = serial.tools.list_ports.comports()[0].device

    if port_name == "None" and args.portname == "None":
        port_name = detect_port()

    if args.portname != "None":
        port_name = args.portname

    if args.baud_rate:
        if int(args.baud_rate) < 9600:
            sys.stderr.write("Failed to set baudrate, minimum is 9600\n")
            sys.exit(1)
        elif int(args.baud_rate) > 921600:
            sys.stderr.write("Failed to set baudrate, maximum is 921600\n")
            sys.exit(1)
        else:
            br = args.baud_rate
    else:
        br = 115200

    if args.debug:
        print("Entering interactive debug mode")
        cmd = "python -m serial.tools.miniterm --raw " + port_name + " " + str(br)
        os.system(cmd)

    if args.wait_flag:
        print("Waiting for serial port", port_name, br)
        serial_port = wait_for_serial_port(port_name, br)
    else:
        print("Using serial port", port_name, br)
        serial_port = open_serial_port(port_name, br)

    if args.get_id_flag:
        serial_port = capture_bootloader(serial_port, port_name, br)
        get_id(serial_port)
        sys.exit(0)

    if args.get_regcode_flag:
        serial_port = capture_bootloader(serial_port, port_name, br)
        get_regcode(serial_port)
        sys.exit(0)

    if args.get_version_flag:
        serial_port = capture_bootloader(serial_port, port_name, br)
        get_version(serial_port)
        sys.exit(0)

    update_commands = []
    if args.system_image_name:
        if is_merged_binary(args.system_image_name):
            append_merged_files(args.system_image_name, update_commands)
        else:
            update_commands.append(
                ["a%x" % FIRMWARE_START_ADDRESS, args.system_image_name, None]
            )
    if args.user_app_name:
        if is_merged_binary(args.user_app_name):
            append_merged_files(args.user_app_name, update_commands)
        else:
            update_commands.append(["s", args.user_app_name, None])
    if args.network_info_bin_name:
        update_commands.append(["o", args.network_info_bin_name, None])
    if args.merged_bin_name:
        if is_merged_binary(args.merged_bin_name):
            append_merged_files(args.merged_bin_name, update_commands)
        else:
            sys.stderr.write("Failed to extract files\n")
            sys.exit(1)
    if args.raw_commands is not None:
        args.raw_commands[0].extend([None])
        update_commands += args.raw_commands
    if args.test_image_name:
        update_commands.append(
            ["a%x" % FIRMWARE_START_ADDRESS, args.test_image_name, None]
        )

    if update_commands:
        serial_port = capture_bootloader(serial_port, port_name, br)
        for d in update_commands:
            if update_image(serial_port, d[0], d[1], d[2]):
                print("done", end="")
            else:
                sys.stderr.write("Update failed!\n")
                serial_port.close()
                sys.exit(1)
        print("\nUpdate done!\n")

    if args.start_flag:
        print("Starting the application")
        jump_to_app(serial_port)

    if args.listen_port_flag:
        dump_debug_output(serial_port)

    serial_port.close()


if __name__ == "__main__":
    main()
