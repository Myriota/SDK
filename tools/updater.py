#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2016-2024, Myriota Pty Ltd, All Rights Reserved
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


import sys
import serial
import serial.tools.list_ports
from serial.tools.list_ports_common import ListPortInfo
import argparse
import os
import signal
import sys
import time
import struct
import tempfile
import platform
from typing import Callable, List, Optional

version = "1.4"


FIRMWARE_START_ADDRESS = 0x4000
header_length = 16
header_version = 0


FILE_TYPE_SYS_IMG = 1
FILE_TYPE_USER_APP = 2
FILE_TYPE_NETWORK_INFO = 3
FILE_TYPE_SYS_IMG_2 = 4


file_types = {
    FILE_TYPE_SYS_IMG: "system image",
    FILE_TYPE_USER_APP: "user application",
    FILE_TYPE_NETWORK_INFO: "network information",
    FILE_TYPE_SYS_IMG_2: "system image part 2",
}


COMMAND_AUTOBAUD = b"U"
COMMAND_BOOT = b"b"
COMMAND_VERSION = b"V"
COMMAND_REGCODE = b"g"
COMMAND_ID = b"i"


class MyriotaUSBDevice:
    DisplayName = ""
    ChipType = ""
    VID = None
    PID = None
    interfaceNum = None

    def __init__(self, DisplayName, ChipType, VID, PID, interfaceNum):
        self.DisplayName = DisplayName
        self.ChipType = ChipType
        self.VID = VID
        self.PID = PID
        self.interfaceNum = interfaceNum

    def __eq__(self, other):
        return all(
            [
                self.DisplayName == other.DisplayName,
                self.ChipType == other.ChipType,
                self.VID == other.VID,
                self.PID == other.PID,
                self.interfaceNum == other.interfaceNum,
            ]
        )


CHIP_TYPE_MODULE = "Module"
CHIP_TYPE_BLE = "BLE"

DEV_UNKNOWN = MyriotaUSBDevice(
    DisplayName="Unknown",
    ChipType=None,
    VID=None,
    PID=None,
    interfaceNum=None,
)
DEV_MODULE_A = MyriotaUSBDevice(
    DisplayName="Myriota Module",
    ChipType=CHIP_TYPE_MODULE,
    VID=0x0403,
    PID=0x6001,
    interfaceNum=None,
)
DEV_MODULE_B = MyriotaUSBDevice(
    DisplayName="Myriota Module",
    ChipType=CHIP_TYPE_MODULE,
    VID=0x0403,
    PID=0x6010,
    interfaceNum=None,
)
DEV_MODULE_C = MyriotaUSBDevice(
    DisplayName="Myriota Module",
    ChipType=CHIP_TYPE_MODULE,
    VID=0x0403,
    PID=0x6011,
    interfaceNum=None,
)
DEV_MODULE_D = MyriotaUSBDevice(
    DisplayName="Myriota Module",
    ChipType=CHIP_TYPE_MODULE,
    VID=0x0403,
    PID=0x6014,
    interfaceNum=None,
)
DEV_MODULE_E = MyriotaUSBDevice(
    DisplayName="Myriota Module",
    ChipType=CHIP_TYPE_MODULE,
    VID=0x0403,
    PID=0x6015,
    interfaceNum=None,
)
DEV_FLEXSENSE_MM = MyriotaUSBDevice(
    DisplayName="Myriota FlexSense Module",
    ChipType=CHIP_TYPE_MODULE,
    VID=0x2FE3,
    PID=0x4D79,
    interfaceNum=0,
)
DEV_FLEXSENSE_BT = MyriotaUSBDevice(
    DisplayName="Myriota FlexSense BLE Adapter",
    ChipType=CHIP_TYPE_BLE,
    VID=0x2FE3,
    PID=0x4D79,
    interfaceNum=3,
)
DEV_KITS = [DEV_MODULE_A, DEV_MODULE_B, DEV_MODULE_C, DEV_MODULE_D, DEV_MODULE_E]
PROGRAMMABLE_DEVICES = [DEV_UNKNOWN, DEV_FLEXSENSE_MM] + DEV_KITS
NON_PROGRAMMABLE_DEVICES = [DEV_FLEXSENSE_BT]
ALL_DEVICES = PROGRAMMABLE_DEVICES + NON_PROGRAMMABLE_DEVICES


def bytecrc(crc, poly, n):
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
    table = [bytecrc(i << (bit_size - 8), poly, bit_size) for i in range(256)]
    crc = 0
    for b in data:
        crc = table[b ^ ((crc >> 8) & 0xFF)] ^ ((crc << 8) & 0xFF00)
    return crc


class MyriotaModuleUpdate:
    serial_port = None

    def __init__(
        self,
        connect_msg: Callable[..., str],
        update_msg: Callable[..., str],
        tx_progress: Callable[..., int],
        module_output: Callable[..., str],
    ):
        self.connect_msg = connect_msg
        self.update_msg = update_msg
        self.tx_progress = tx_progress
        self.module_output = module_output

    def reset_device(self):
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
                    self.connect_msg("Please reset the device", end="")
                    return

            self.connect_msg("Resetting Myriota device", end="")
            d.setBitMode(0xF0, 0x20)  # set reset pin low
            time.sleep(0.1)
            d.setBitMode(0xF1, 0x20)  # set reset pin high
            time.sleep(0.1)
            d.setBitMode(0, 0x20)
            d.close()
        except (OSError, ImportError) as e:
            self.connect_msg("Please reset the device", end="")
            return

    def _get_interface_num(self, port_info: ListPortInfo) -> int:
        if platform.system() == "Darwin":
            try:
                p_interfaceNum = int(port_info.device[-1]) - 1
                return p_interfaceNum
            except (IndexError, ValueError):
                return 0
        else:
            if port_info.location is None:
                return 0
            else:
                try:
                    p_interfaceNum = int(port_info.location.split(".")[-1])
                    return p_interfaceNum
                except (IndexError, ValueError):
                    return 0

    def _get_ports_per_usb(self, usb_dev: MyriotaUSBDevice):
        if usb_dev == DEV_UNKNOWN:
            return {}

        all_ports = serial.tools.list_ports.comports()
        found = {}

        for p in all_ports:
            if p.vid != usb_dev.VID or p.pid != usb_dev.PID:
                continue

            if usb_dev.interfaceNum is not None:
                p_interfaceNum = self._get_interface_num(p)
                if p_interfaceNum != usb_dev.interfaceNum:
                    continue
                if p.location is None:
                    if usb_dev.interfaceNum != 0:
                        continue
                elif p.location is not None:
                    try:
                        p_interfaceNum = int(p.location.split(".")[-1])
                    except (IndexError, ValueError):
                        pass

            found[p.device] = usb_dev
        return found

    def get_ports(
        self,
        ignore_list: Optional[List[MyriotaUSBDevice]] = [],
        usb_info: List[MyriotaUSBDevice] = ALL_DEVICES,
    ):
        # Returns dict of {port_name: MyriotaUSBDevice}
        ports_usb_desc = {}
        for usb_dev in usb_info:
            desc = self._get_ports_per_usb(usb_dev)
            ports_usb_desc = {**ports_usb_desc, **desc}

        ports = {}
        for port in serial.tools.list_ports.comports():
            port_usb_info = (
                ports_usb_desc[port.device]
                if port.device in ports_usb_desc
                else DEV_UNKNOWN
            )
            if ignore_list and port_usb_info in ignore_list:
                continue
            ports[port.device] = port_usb_info

        return ports

    def get_port_usable(self):
        ports = self.get_ports(ignore_list=NON_PROGRAMMABLE_DEVICES)
        return set(ports.keys())

    def detect_port(self):
        self.connect_msg(
            "Please connect/reconnect the USB cable ",
            end="",
        )
        existing_ports = self.get_port_usable()
        while len(self.get_port_usable().difference(existing_ports)) == 0:
            existing_ports = self.get_port_usable()
            time.sleep(0.5)
            self.connect_msg(".", end="")
        self.connect_msg("")
        ret = self.get_port_usable().difference(existing_ports).pop()
        return ret

    def open_serial_port(self, portname, baudrate):
        try:
            self.serial_port = serial.Serial(
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
            raise RuntimeError("Failed to open %s\n" % portname)

    def wait_for_serial_port(self, portname, baudrate):
        self.serial_port = None
        while self.serial_port is None:
            try:
                self.serial_port = serial.Serial(
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
                time.sleep(0.5)
                pass

    def execute_cmd(self, cmd: bytearray):
        if self.serial_port is None:
            raise RuntimeError("no serial port open")
        self.serial_port.write(cmd)
        self.serial_port.flush()

    def execute_cmd_read(self, cmd: bytearray, max_retries=2):
        retires = 0
        while retires < max_retries:
            retires += 1

            self.serial_port.reset_input_buffer()
            self.execute_cmd(cmd)

            out = self.serial_port.readline()
            if out.strip(b"\r\n") == cmd.strip(b"\r\n") or not out.strip(b"\r\n"):
                # skip command echo/empty response
                out = self.serial_port.readline()

            if b"Unknown" in out or not out.strip(b"\r\n"):
                # bootloader response invalid or still no response
                continue
            else:
                return out.decode("utf-8")
        raise TimeoutError(out)

    def capture_bootloader(self, port_name, br):
        # return straightaway if already in bootloader
        if self.serial_port is None:
            raise RuntimeError("no serial port open")
        self.serial_port.reset_input_buffer()

        autobaud_retires = 3
        while autobaud_retires > 0:
            self.execute_cmd(COMMAND_AUTOBAUD)
            out = self.serial_port.readline()
            if out is not None:
                out += self.serial_port.readline()
                out += self.serial_port.readline()
            if b"Unknown" in out or b"Bootloader" in out:
                self.serial_port.readline()
                self.serial_port.readline()
                return self.serial_port
            time.sleep(0.5)
            autobaud_retires -= 1
        self.serial_port.close()

        # reset Myriota device
        self.reset_device()

        self.open_serial_port(port_name, br)
        self.serial_port.reset_input_buffer()

        # Try to capture the bootloader
        MAX_RETRIES = 10
        retries = 0
        while True:
            self.execute_cmd(COMMAND_AUTOBAUD)
            line = self.serial_port.readline()
            out = line
            if b"Bootloader" in line:
                self.connect_msg("\n")
                self.serial_port.readline()
                self.serial_port.readline()
                self.serial_port.readline()
                self.serial_port.readline()
                break
            if line is not None:
                out += self.serial_port.readline()
                if b"Unknown" in out:
                    break
            retries += 1
            if retries > MAX_RETRIES:
                raise RuntimeError("failed to connect to the board")
            time.sleep(0.5)
            self.connect_msg(".", end="")

        self.serial_port.reset_input_buffer()

    def close(self):
        if self.serial_port is not None:
            self.serial_port.close()

    def _xmodem_send(self, file, ncg, quiet=True):
        SOH = bytes(bytearray([0x01]))
        EOT = bytes(bytearray([0x04]))
        ACK = bytes(bytearray([0x06]))
        NAK = bytes(bytearray([0x15]))
        NCG = b"C"
        DATA_SIZE = 128
        if not ncg:
            t = 0
            while True:
                if self.serial_port.read(1) != NCG:
                    t = t + 1
                    if t == 10:
                        self.update_msg("*", end="")
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
                self.serial_port.write(SOH)
                self.serial_port.write(bytes(bytearray([pn])))
                self.serial_port.write(bytes(bytearray([0xFF - pn])))
                self.serial_port.write(data)
                self.serial_port.write(bytes(bytearray([(crc & 0xFF00) >> 8])))
                self.serial_port.write(bytes(bytearray([crc & 0xFF])))
                self.serial_port.flush()
                answer = self.serial_port.read(1)
                if answer == NAK:
                    if not quiet:
                        self.update_msg("!", end="")
                    retries += 1
                    if retries > MAX_RETRIES:
                        return False
                    else:
                        continue
                if answer == ACK:
                    retries = 0
                    tx_size += DATA_SIZE
                    self.tx_progress(tx_size)
                    break
                # If got nothing, exit
                self.serial_port.write(EOT)
                self.serial_port.flush()
                if not quiet:
                    self.update_msg("$", end="")
                return False
            data = bytearray(file.read(DATA_SIZE))
            pn = (pn + 1) % 256
        self.serial_port.write(EOT)
        self.serial_port.flush()
        answer = self.serial_port.read(1)
        if answer == NAK:
            return False
        return True

    def _update_stream(self, command, stream):
        retries = 3
        update_in_progress = False
        while True:
            stream.seek(0)
            self.serial_port.reset_input_buffer()
            self.serial_port.write(bytearray(command.encode("utf-8")))
            self.serial_port.flush()
            out = b""
            out += self.serial_port.readline()
            out += self.serial_port.readline()
            out += self.serial_port.readline()
            out += self.serial_port.readline()
            ncg = False
            if b"C" in out:
                ncg = True
            else:
                if not update_in_progress and b"Fail" in out and retries <= 1:
                    raise RuntimeError(
                        "partition error: Please confirm if correct system image has been programmed"
                    )
            if b"Ready" in out and not b"Fail" in out:
                update_in_progress = True
                if self._xmodem_send(stream, ncg):
                    stream.close()
                    return
            retries -= 1
            if retries == 0:
                break
            else:
                time.sleep(1)

        raise RuntimeError("Could not update module")

    def get_xmodem_file_size(self, stream):
        stream.seek(0, os.SEEK_END)
        file_size = stream.tell()
        return file_size

    def update_image(self, command, filename, handle=None):
        if self.serial_port is None:
            raise RuntimeError("no serial port open")

        try:
            if handle is None:
                stream = open(filename, "rb")
            else:
                stream = handle
        except IOError:
            raise RuntimeError("Can't open %s" % filename)

        file_size = self.get_xmodem_file_size(stream)
        stream.seek(0, os.SEEK_SET)
        self.update_msg(
            "\nProgramming %s (%dK) " % (filename, (file_size + 1023) / 1024),
            end="",
        )
        self._update_stream(command, stream)
        self.update_msg("done")

    def jump_to_app(self):
        self.execute_cmd(COMMAND_BOOT)

    def get_id(self):
        try:
            return self.execute_cmd_read(COMMAND_ID, max_retries=2)
        except TimeoutError:
            raise RuntimeError("Failed to read ID")

    def get_regcode(self):
        try:
            return self.execute_cmd_read(COMMAND_REGCODE, max_retries=2)
        except TimeoutError:
            raise RuntimeError("Failed to read regcode")

    def get_version(self):
        try:
            return self.execute_cmd_read(COMMAND_VERSION, max_retries=2)
        except TimeoutError as e:
            return str(e)

    def dump_debug_output(self):
        if self.serial_port is None:
            raise RuntimeError("no serial port open")
        while True:
            out = self.serial_port.readline()
            if len(out) != 0:
                self.module_output(out.decode("utf-8"), end="")

    def is_merged_binary(self, filename):
        # Let caller handle exceptions
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

    def append_merged_files(self, filename, command):
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
                if ftype == FILE_TYPE_SYS_IMG:
                    command.append(
                        [
                            "a%x" % FIRMWARE_START_ADDRESS,
                            filename + "(" + file_types.get(ftype) + ")",
                            output_temp,
                        ]
                    )
                if ftype == FILE_TYPE_USER_APP:
                    command.append(
                        [
                            "s",
                            filename + "(" + file_types.get(ftype) + ")",
                            output_temp,
                        ]
                    )
                if ftype == FILE_TYPE_NETWORK_INFO:
                    command.append(
                        [
                            "o",
                            filename + "(" + file_types.get(ftype) + ")",
                            output_temp,
                        ]
                    )
                if ftype == FILE_TYPE_SYS_IMG_2:
                    command.append(
                        [
                            "S",
                            filename + "(" + file_types.get(ftype) + ")",
                            output_temp,
                        ]
                    )
                output_temp.flush()
                offset += flen + header_length


updater = None


def signal_handler(signal, frame):
    if updater is not None and updater.serial_port is not None:
        updater.serial_port.close()
    sys.exit(0)


def stdoutprint(*objects, end="\n"):
    print(*objects, end=end)
    sys.stdout.flush()


def tx_progress(tx_size):
    if not tx_size % (1024 * 10):
        print(".", end="")
        sys.stdout.flush()


def main():
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

    parser.add_argument(
        "-k",
        "--list-ports",
        dest="list_ports_flag",
        action="store_true",
        default=False,
        help="List all connected serial ports",
    )

    args = parser.parse_args()

    global updater
    updater = MyriotaModuleUpdate(
        connect_msg=stdoutprint,
        update_msg=stdoutprint,
        tx_progress=tx_progress,
        module_output=stdoutprint,
    )

    if args.list_ports_flag:
        ports = updater.get_ports()
        max_port_len = max([len(n) for n in ports.keys()], default=0) + 2
        print("{:<{}} {}".format("Port", max_port_len, "Description"))
        for port_name, myriota_usb_info in ports.items():
            description = myriota_usb_info.DisplayName
            print("{:<{}} {}".format(port_name, max_port_len, description))
        sys.exit(0)

    if args.default_port_flag:
        if serial.tools.list_ports.comports():
            port_name = serial.tools.list_ports.comports()[0].device

    if port_name == "None" and args.portname == "None":
        port_name = updater.detect_port()

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
        updater.wait_for_serial_port(port_name, br)
    else:
        print("Using serial port", port_name, br)
        try:
            updater.open_serial_port(port_name, br)
        except Exception as e:
            sys.stderr.write(str(e) + "\n")
            sys.exit(1)

    if args.get_id_flag:
        try:
            updater.capture_bootloader(port_name, br)
            print("ID:", end="")
            print(updater.get_id())
            sys.exit(0)
        except Exception as e:
            sys.stderr.write(str(e))
            sys.exit(1)

    if args.get_regcode_flag:
        try:
            updater.capture_bootloader(port_name, br)
            print("Registration code:", end="")
            print(updater.get_regcode())
            sys.exit(0)
        except Exception as e:
            sys.stderr.write(str(e))
            sys.exit(1)

    if args.get_version_flag:
        try:
            updater.capture_bootloader(port_name, br)
            print(updater.get_version())
            sys.exit(0)
        except Exception as e:
            sys.stderr.write(str(e))
            sys.exit(1)

    update_commands = []
    if args.system_image_name:
        try:
            if updater.is_merged_binary(args.system_image_name):
                updater.append_merged_files(args.system_image_name, update_commands)
            else:
                update_commands.append(
                    ["a%x" % FIRMWARE_START_ADDRESS, args.system_image_name, None]
                )
        except IOError:
            sys.stderr.write("\nCan't open %s\n" % args.system_image_name)
            sys.exit(1)

    if args.user_app_name:
        try:
            if updater.is_merged_binary(args.user_app_name):
                updater.append_merged_files(args.user_app_name, update_commands)
            else:
                update_commands.append(["s", args.user_app_name, None])
        except IOError:
            sys.stderr.write("\nCan't open %s\n" % args.user_app_name)
            sys.exit(1)

    if args.network_info_bin_name:
        update_commands.append(["o", args.network_info_bin_name, None])

    if args.merged_bin_name:
        try:
            if updater.is_merged_binary(args.merged_bin_name):
                updater.append_merged_files(args.merged_bin_name, update_commands)
            else:
                sys.stderr.write("Failed to extract files\n")
                sys.exit(1)
        except IOError:
            sys.stderr.write("\nCan't open %s\n" % args.merged_bin_name)
            sys.exit(1)

    if args.raw_commands is not None:
        args.raw_commands[0].extend([None])
        update_commands += args.raw_commands

    if args.test_image_name:
        update_commands.append(
            ["a%x" % FIRMWARE_START_ADDRESS, args.test_image_name, None]
        )

    if update_commands:
        try:
            updater.capture_bootloader(port_name, br)
            for d in update_commands:
                updater.update_image(d[0], d[1], d[2])
            print("\nUpdate done!\n")
        except Exception as e:
            updater.close()
            sys.stderr.write("updated failed\n")
            sys.stderr.write(str(e) + "\n")
            sys.stderr.flush()
            sys.exit(1)

    if args.start_flag:
        try:
            print("Starting the application")
            updater.jump_to_app()
        except Exception as e:
            sys.stderr.write(str(e) + "\n")
            sys.stderr.flush()
            sys.exit(1)

    if args.listen_port_flag:
        try:
            print("Dumping debug output")
            updater.dump_debug_output()
        except Exception as e:
            sys.stderr.write(str(e) + "\n")
            sys.stderr.flush()
            sys.exit(1)

    updater.close()


if __name__ == "__main__":
    main()
