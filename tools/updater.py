#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2016-2019, Myriota Pty Ltd, All Rights Reserved
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
import argparse
import os


def open_serial_port(portname):
    try:
        ser = serial.Serial(
            port=portname,
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            xonxoff=0,
            rtscts=0,
            timeout=0.5
        )
    except (OSError, serial.SerialException):
        print('Failed to open', portname)
        sys.exit(1)
    return ser


def capture_bootloader(ser):
    # return straightaway if already in bootloader
    ser.reset_input_buffer()
    ser.write(b'U')
    ser.flush()
    out = ser.readline()
    if out is not None:
        out += ser.readline()
        out += ser.readline()
    if b'Unknown' in out or b'Bootloader' in out:
        return
    print('Please reset the board', end='')
    sys.stdout.flush()
    ser.reset_input_buffer()

    # Try to capture the bootloader
    MAX_RETRIES = 10
    retries = 0
    while True:
        ser.write(b'U')
        ser.flush()
        line = ser.readline()
        out = line
        if b'Bootloader' in line:
            print('\n', line)
            ser.readline()
            break
        if line is not None:
            out += ser.readline()
            if b'Unknown' in out:
                break
        retries += 1
        if retries > MAX_RETRIES:
            sys.stderr.write('failed to connect to the board\n')
            sys.exit(1)
        time.sleep(0.5)
        print('.', end='')
        sys.stdout.flush()

    ser.reset_input_buffer()


def calc_crc(data):
    crc = 0
    for b in data:
        crc = crc ^ (b << 8)
        for i in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc <<= 1
    return crc


def xmodem_send(serial, file, quiet=True):
    SOH = bytes(bytearray([0x01]))
    EOT = bytes(bytearray([0x04]))
    ACK = bytes(bytearray([0x06]))
    NAK = bytes(bytearray([0x15]))
    NCG = b'C'
    DATA_SIZE = 128

    t = 0
    while True:
        if serial.read(1) != NCG:
            t = t + 1
            if t == 10:
                print('*', end='')
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
                    print('!', end='')
                    sys.stdout.flush()
                retries += 1
                if retries > MAX_RETRIES:
                    return False
                else:
                    continue
            if answer == ACK:
                retries = 0
                tx_size += DATA_SIZE
                if not tx_size % (1024*10):
                    print('.', end='')
                    sys.stdout.flush()
                break
            # If got nothing, exit
            serial.write(EOT)
            serial.flush()
            if not quiet:
                print('$', end='')
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


def update_image(ser, command, filename):
    try:
        stream = open(filename, 'rb')
    except IOError:
        sys.stderr.write('failed to open the file\n')
        return False
    print('\nProgramming %s (%dK) '
          % (filename, (os.path.getsize(filename)+1023)/1024), end='')
    sys.stdout.flush()
    retries = 3
    while True:
        stream.seek(0)
        ser.reset_input_buffer()
        ser.write(bytearray(command.encode('utf-8')))
        ser.flush()
        out = b''
        out += ser.readline()
        out += ser.readline()
        if b'Ready' in out:
            if xmodem_send(ser, stream):
                stream.close()
                return True
        retries -= 1
        if retries == 0:
            break
        else:
            time.sleep(1)

    sys.stderr.write('failed\n')
    return False


def jump_to_app(ser):
    ser.write(b'b')
    ser.flush()


def get_id(ser):
    print('ID:',end='')
    ser.write(b'i')
    ser.flush()
    out = b''
    ser.readline()
    out += ser.readline()
    print(out.decode('utf-8'))

def get_regcode(ser):
    print('Registration code:',end='')
    ser.write(b'g')
    ser.flush()
    out = b''
    ser.readline()
    out += ser.readline()
    print(out.decode('utf-8'))

ser = None


def main():

    port_name = 'None'
    import serial.tools.list_ports
    if serial.tools.list_ports.comports():
        port_name = serial.tools.list_ports.comports()[0].device

    parser = argparse.ArgumentParser(description='Myriota terminal updater',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-f", "--system_image", dest='system_image_name',
                        help='system image FILE to update with', metavar='FILE')
    parser.add_argument("-u", "--user_app", dest='user_app_name',
                        help='uaer application FILE to update with', metavar='FILE')
    parser.add_argument('-r', '--raw', nargs=2, dest='raw_commands',
                        action='append', metavar=('raw_command', 'FILE'),
                        help='use raw command and FILE pair to update, can have multiple')
    parser.add_argument('-p', '--port', dest='portname', metavar='PORT', default=port_name,
                        help='serial PORT the Myriota terminal is connected to, e.g. /dev/ttyUSB0')
    parser.add_argument('-s', '--start', dest='start_flag', action='store_true',
                        default=False,
                        help='start the application after update')
    parser.add_argument('-i', '--id', dest='get_id_flag', action='store_true',
                        default=False,
                        help='get ID')
    parser.add_argument('-c', '--regcode', dest='get_regcode_flag', action='store_true',
                        default=False,
                        help='get registration code')
    args = parser.parse_args()
    if args.portname == 'None':
        parser.error("Please specify the serial port.")
    port_name = args.portname

    print('Using serial port', port_name)
    serial_port = open_serial_port(port_name)
    capture_bootloader(serial_port)

    if args.get_id_flag:
        get_id(serial_port)
        sys.exit(0)

    if args.get_regcode_flag:
        get_regcode(serial_port)
        sys.exit(0)

    update_commands = []
    if args.system_image_name:
        update_commands.append(['u', args.system_image_name])
    if args.user_app_name:
        update_commands.append(['s', args.user_app_name])
    if args.raw_commands is not None:
        update_commands += args.raw_commands

    if not update_commands:
        parser.error("Please specify the files to program.")

    for d in update_commands:
        if update_image(serial_port, d[0], d[1]):
            print('done')
        else:
            serial_port.close()
            sys.exit(1)

    if args.start_flag:
        print('Starting the application')
        jump_to_app(serial_port)

    serial_port.close()


if __name__ == "__main__":
    main()
