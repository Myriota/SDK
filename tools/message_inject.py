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


import myriota_auth
from datetime import datetime
import json
import requests
import sys
import time

_domain = "https://api.myriota.com/v1"


def do_inject(idtoken, moduleid):
    print("Injecting messages from %s..." % moduleid)
    line = ""
    while True:
        ch = sys.stdin.read(1)
        if ch == "":
            print("Exiting...")
            exit(0)
        line += ch
        if line.endswith("\n"):
            if line.strip():
                message = line.strip().rstrip("\n")
                print(message)
                data = {"TerminalId": moduleid, "Message": message}
                headers = {
                    "Content-type": "application/json",
                    "Authorization": idtoken()["IdToken"],
                }
                url = _domain + "/messages"
                try:
                    response = requests.post(
                        url, data=json.dumps(data), headers=headers
                    )
                    response.raise_for_status()
                except requests.exceptions.RequestException as e:
                    raise SystemExit(e)
            line = ""


def main(argv=None):
    """CLI entrypoint"""
    import getpass
    import argparse
    import os
    import sys

    parser = argparse.ArgumentParser(
        description='Command line interface for manually injecting message. Messages are input using stdin. Example: echo "1234abcd" | ./message_inject.py <moduleid>'
    )
    parser.add_argument("moduleid", help="Module Id")
    args = parser.parse_args(argv)

    try:
        myriota_auth.auth_token(myriota_auth.get_cached_token())
    except (IOError, ValueError):
        print("Run myriota_auth.py to generate security token first.")
        return
    idtoken = myriota_auth.auto_auth()
    try:
        do_inject(idtoken, args.moduleid)
    except requests.exceptions.HTTPError as e:
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
