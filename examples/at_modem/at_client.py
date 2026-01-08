#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2025, Myriota Pty Ltd, All Rights Reserved
# SPDX-License-Identifier: BSD-3-Clause-Attribution
#
# This file is licensed under the BSD with attribution  (the "License"); you
# may not use these files except in compliance with the License.
#
# You may obtain a copy of the License here:
# LICENSE-BSD-3-Clause-Attribution.txt and at
# https://spdx.org/licenses/BSD-3-Clause-Attribution.html
#
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import time
import signal
import argparse
from datetime import datetime
import struct

import requests
import serial
import serial.tools.list_ports

VERSION = "1.0"


# -------------------------------
# Context container
# -------------------------------
class Context:
    def __init__(self):
        self.serial_port = None
        self.skip_gnss = False


# -------------------------------
# Serial port operations
# -------------------------------
def list_serial_ports():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return
    print("Available serial ports:")
    for port in ports:
        print(
            f"- {port.device}\n  Description: {port.description}\n"
            f"  Manufacturer: {port.manufacturer}\n  Product: {port.product}\n"
        )


def connect_to_port(port, baudrate):
    try:
        return serial.Serial(
            port,
            baudrate,
            serial.EIGHTBITS,
            serial.PARITY_NONE,
            serial.STOPBITS_ONE,
            timeout=1,
        )
    except serial.SerialException as e:
        print(f"Error connecting to port {port}: {e}")
        return None


def detect_port():
    def get_ports():
        return {p.device for p in serial.tools.list_ports.comports()}

    print("Please connect/reconnect the USB cable...")
    while True:
        before = get_ports()
        time.sleep(0.5)
        current = get_ports()
        diff = current - before
        if diff:
            new_port = diff.pop()
            print(f"Port detected: {new_port}")
            return new_port


def validate_baudrate(baud):
    try:
        baud = int(baud)
        if 9600 <= baud <= 921600:
            return baud
    except ValueError:
        pass
    sys.stderr.write("Invalid baudrate. Must be between 9600 and 921600.\n")
    sys.exit(1)


# -------------------------------
# Response / Read functions
# -------------------------------
def monitor_response(ctx, delay=0.1, timeout=2):
    response, start_time = "", time.perf_counter()
    try:
        while time.perf_counter() - start_time < timeout:
            if ctx.serial_port.in_waiting:
                time.sleep(delay)
                response += ctx.serial_port.read(ctx.serial_port.in_waiting).decode()
                print(f"Received: {response.strip()}")
                ctx.serial_port.reset_input_buffer()
                return response.strip()
            time.sleep(0.1)
    except Exception as e:
        print(f"Error while monitoring response: {e}")
    return None


def send_command_with_response(ctx, command, expected, extract_int=False):
    ctx.serial_port.write(f"{command}\n".encode())
    print(f"Sent: {command}")
    resp = monitor_response(ctx)
    if resp and expected in resp:
        return int(resp.split("=")[1]) if extract_int else resp
    return None


def read_module_state(ctx):
    print("\nChecking Modem State...")
    return send_command_with_response(ctx, "AT+STATE=?", "OK+STATE")


def read_module_time(ctx):
    print("\nReading Module Time...")
    return send_command_with_response(ctx, "AT+TIME=?", "OK+TIME", extract_int=True)


def read_module_location(ctx):
    print("\nReading Module Location...")
    resp = send_command_with_response(ctx, "AT+LOCATION=?", "OK+LOCATION")
    if resp:
        lat, lon = map(int, resp.split("=")[1].split(","))
        return {"latitude": lat, "longitude": lon}
    return None


# -------------------------------
# Set / Control functions
# -------------------------------
def send_control_cmd(ctx, cmd, param=None):
    formatted = f"AT+{cmd}={param}\n" if param else f"AT+{cmd}\n"
    ctx.serial_port.write(formatted.encode())
    print(f"Sent: {formatted}")


def execute_with_retries(attempts, func, *args, **kwargs):
    for attempt in range(1, attempts + 1):
        try:
            if func(*args, **kwargs):
                return True
        except Exception as e:
            print(f"Attempt {attempt} failed: {e}")
        time.sleep(3)
    print(f"Failed after {attempts} attempts.")
    return False


def set_time(ctx):
    print("\nSetting Time...")

    def do_set_time():
        send_control_cmd(ctx, "TIME", int(time.time()))
        response = monitor_response(ctx)
        return response and "OK+TIME" in response

    execute_with_retries(3, do_set_time)


def get_host_location():
    try:
        loc = requests.get("https://ipinfo.io/json").json().get("loc")
        return map(float, loc.split(",")) if loc else (None, None)
    except Exception as e:
        print(f"Error fetching location: {e}")
        return None, None


def set_location(ctx):
    print("\nSetting Location...")

    def do_set_location():
        lat, lon = get_host_location()
        if lat is None or lon is None:
            raise ValueError("Failed to fetch current location.")
        send_control_cmd(ctx, "LOCATION", f"{int(lat * 1e7)},{int(lon * 1e7)}")
        response = monitor_response(ctx)
        return response and "OK+LOCATION" in response

    execute_with_retries(3, do_set_location)


# -------------------------------
# Tracker functions
# -------------------------------
def handle_gnss_fix(ctx):
    print("\nGetting GNSSFIX. The modem may temporarily become unresponsive...")
    send_control_cmd(ctx, "GNSSFIX")

    response = monitor_response(ctx)
    if response:
        ok_count = response.count("OK+GNSSFIX")

        if ok_count == 2:
            print(
                "GNSSFIX skipped. Consider using '--skip-gnss' option with GNSS skipped modem firmware."
            )

        elif ok_count == 1:
            print("First GNSSFIX response received.")
            print("Waiting for second GNSSFIX response or timeout (100s)...")

            start_time = time.time()
            while time.time() - start_time <= 100:
                additional_resp = monitor_response(ctx)
                if additional_resp:
                    if "OK+GNSSFIX" in additional_resp:
                        print("GNSSFIX succeeded.")
                        break
                    elif "FAIL+GNSSFIX" in additional_resp:
                        print("GNSSFIX failed.")
                        break
            else:
                print("GNSSFIX timed out.")

        else:
            print("Unexpected response received. No GNSSFIX status detected.")

    else:
        print("No response received from modem.")

    ctx.serial_port.reset_input_buffer()


def schedule_message(ctx, seq, mpd):
    # Fetch module time and location
    module_time = read_module_time(ctx)
    loc = read_module_location(ctx)
    if not module_time or not loc:
        print("Failed to retrieve module time or location.")
        print("Message is not scheduled.")
    else:
        print("\nScheduling Message...")
        payload = struct.pack(
            "<HBiiI", seq, 1, loc["latitude"], loc["longitude"], int(module_time)
        )
        print(f"Message Payload: {payload.hex().upper()}")
        print(f"Sequence Number: {seq}")
        print(f"Location Count: {1}")
        print(f"Latitude: {loc['latitude'] / 1e7}")
        print(f"Longitude: {loc['longitude'] / 1e7}")
        print(f"Time: {int(module_time)}")

        send_control_cmd(ctx, "SMSG", payload.hex().upper())
        response = monitor_response(ctx)
        if response and "OK+SMSG" in response:
            print(f"Message scheduled at {datetime.fromtimestamp(module_time)}.")
        else:
            print("Message scheduling failed.")

    # Compute next scheduling time
    next_time = int(time.time()) + int((24 / mpd) * 3600)
    print(f"Next message scheduling at {datetime.fromtimestamp(next_time)}.")


def tracker_mode(ctx, msgs_per_day):
    # Pre-compute constants
    seconds_per_msg = 24 * 3600 / msgs_per_day
    time_sync_interval_sec = 12 * 3600  # 12h

    gnss_mode = "skipped" if ctx.skip_gnss else "active"
    print(f"Tracker, GNSS fix {gnss_mode}, {msgs_per_day} messages per day.")

    seq = 0
    now = time.time()
    last_sync = now - time_sync_interval_sec
    last_msg_time = now - seconds_per_msg

    while True:
        try:
            now = time.time()

            # Periodic time/location sync
            if ctx.skip_gnss and now - last_sync >= time_sync_interval_sec:
                print(
                    f"\nUpdating time and location every {time_sync_interval_sec // 3600} hours..."
                )
                set_time(ctx)
                set_location(ctx)
                last_sync = now

            # Scheduled message
            if now - last_msg_time >= seconds_per_msg:
                ready = False
                for _ in range(3):
                    resp = read_module_state(ctx)
                    if resp and "READY" in resp:
                        ready = True
                        break

                if not ready:
                    print(
                        "Error! Modem is not ready. Waiting for the next message scheduling window."
                    )
                else:
                    if not ctx.skip_gnss:
                        handle_gnss_fix(ctx)
                    try:
                        schedule_message(ctx, seq, msgs_per_day)
                        seq += 1
                    except Exception as e:
                        print(f"Scheduling failed: {e}")

                last_msg_time = now

                # Clean serial buffer between send attempts
                ctx.serial_port.reset_input_buffer()

            time.sleep(10)

        except Exception as e:
            # Print and continue (no recursion)
            print(f"\nTracker mode error: {e}. Restarting loop...")
            time.sleep(2)


# -------------------------------
# Raw AT command interface
# -------------------------------
def raw_commands(ctx):
    print("Raw AT Command Mode. Type 'exit' or press Ctrl+C to quit.\n")

    BASE_DELAY = 0.1
    PER_CHAR_DELAY = 0.001

    while True:
        cmd = input("Send: ").strip()
        if cmd.lower() == "exit":
            break

        # send command
        ctx.serial_port.write((cmd + "\n").encode())

        # compute dynamic delay
        dynamic_delay = (
            (len(cmd) * PER_CHAR_DELAY + BASE_DELAY)
            if "AT+SMSG=" in cmd
            else BASE_DELAY
        )

        # wait for response
        if not monitor_response(ctx, delay=dynamic_delay):
            print("Timeout.")


# -------------------------------
# System utilities
# -------------------------------
def signal_handler(ctx, sig, frame):
    if ctx.serial_port:
        ctx.serial_port.close()
    sys.exit(0)


def setup_signal_handlers(ctx):
    # Use a lambda function to pass the context object to the signal handler
    signal.signal(signal.SIGINT, lambda s, f: signal_handler(ctx, s, f))
    try:
        signal.signal(signal.SIGPIPE, lambda s, f: signal_handler(ctx, s, f))
    except AttributeError:
        pass


# -------------------------------
# Argument parsing
# -------------------------------
def parse_arguments():
    parser = argparse.ArgumentParser(
        description=f"Myriota AT Modem Host Simulator v{VERSION}",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    # Mutually exclusive operation modes
    mode = parser.add_mutually_exclusive_group(required=True)

    mode.add_argument(
        "-r",
        "--raw",
        action="store_true",
        help="Raw mode: pass data directly without tracker formatting.",
    )

    mode.add_argument(
        "-t",
        "--tracker",
        type=int,
        nargs="?",
        const=3,
        default=None,
        metavar="N",
        help="Tracker mode with N messages per day (default 3).",
    )

    parser.add_argument(
        "-s",
        "--skip-gnss",
        action="store_true",
        help="Use with the skip_gnssfix firmware",
    )

    parser.add_argument(
        "-p",
        "--port",
        default=None,
        help="Serial port (e.g., COM3 or /dev/ttyUSB0). Auto-detect if omitted.",
    )

    parser.add_argument(
        "-b",
        "--baudrate",
        default=9600,
        help="Serial baud rate.",
    )

    parser.add_argument(
        "-l",
        "--list-ports",
        action="store_true",
        help="List available serial ports and exit.",
    )

    return parser.parse_args()


# -------------------------------
# Main entry
# -------------------------------
def main():
    # Instantiate the Context object to hold the shared state
    ctx = Context()

    setup_signal_handlers(ctx)
    args = parse_arguments()

    if args.list_ports:
        list_serial_ports()
        sys.exit(0)

    port = args.port or detect_port()
    baud = validate_baudrate(args.baudrate)

    # Store state in ctx object
    ctx.serial_port = connect_to_port(port, baud)
    ctx.skip_gnss = args.skip_gnss

    if not ctx.serial_port:
        print("No port connected.")
        sys.exit(1)

    if args.raw:
        raw_commands(ctx)
    else:
        tracker_mode(ctx, args.tracker)


if __name__ == "__main__":
    main()
