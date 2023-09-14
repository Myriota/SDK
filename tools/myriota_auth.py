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


import json
import getpass
import requests
import sys
import os
import time

CLIENT_ID = "4jskgo1eq6ngcimlseerg50uvd"
TOKEN_DIR = os.path.expanduser("~") + "/.cache/myriota"
TOKEN_FILE = TOKEN_DIR + "/token"


def post(endpoint, args, region="us-east-1"):
    headers = {
        "Content-Type": "application/x-amz-json-1.1",
        "X-Amz-Target": "AWSCognitoIdentityProviderService.%s" % endpoint,
    }
    url = "https://cognito-idp.%s.amazonaws.com/" % region
    try:
        response = requests.post(url, json=args, headers=headers)
    except requests.exceptions.RequestException as e:
        raise SystemExit(e)
    result = response.json()
    if response.status_code >= 400:
        raise ValueError(
            "; ".join([result["__type"], str(response.status_code), result["message"]])
        )
    return result


def get_username():
    sys.stderr.write("Device manager username: ")
    try:
        return raw_input()  # Try python2 first
    except NameError:
        return input()


def get_password():
    return getpass.getpass("Password: ")


def write_cached_token(token):
    if not os.path.exists(TOKEN_DIR):
        os.makedirs(TOKEN_DIR)
    with open(TOKEN_FILE, "w") as f:
        f.write(token["RefreshToken"])
    os.chmod(TOKEN_FILE, 0o600)


def get_cached_token():
    with open(TOKEN_FILE, "r") as f:
        return f.read().strip()


def clear_cached_token():
    if os.path.exists(TOKEN_FILE):
        os.remove(TOKEN_FILE)


def auth_user_pass(username, password, client_id=CLIENT_ID):
    req = {
        "AuthFlow": "USER_PASSWORD_AUTH",
        "ClientId": client_id,
        "AuthParameters": {"USERNAME": username, "PASSWORD": password},
    }
    response = post("InitiateAuth", req)
    if "ChallengeName" in response:
        if response["ChallengeName"] == "NEW_PASSWORD_REQUIRED":
            raise "Please change your password before authenticating."
        else:
            raise "The %s challenge is not supported" % response["ChallengeName"]
    return response["AuthenticationResult"]


def auth_token(token, client_id=CLIENT_ID):
    req = {
        "AuthFlow": "REFRESH_TOKEN_AUTH",
        "ClientId": client_id,
        "AuthParameters": {"REFRESH_TOKEN": token},
    }
    return post("InitiateAuth", req)["AuthenticationResult"]


def auth(client_id=CLIENT_ID):
    """Authenticate and return temporary token"""
    try:
        return auth_token(get_cached_token(), client_id)
    except (IOError, ValueError):
        token = auth_user_pass(get_username(), get_password(), client_id)
        write_cached_token(token)
        return token


def auto_auth(refresh=3600):
    """Returns a function that returns a periodically refreshed token"""
    s = {"t": auth(), "e": 0}
    s["e"] = time.time() + min(refresh, s["t"]["ExpiresIn"] * 0.9)

    def f():
        if time.time() > s["e"]:
            s["t"] = auth()
            s["e"] = time.time() + min(refresh, s["t"]["ExpiresIn"] * 0.9)
        return s["t"]

    return f


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Myriota device manager authentication CLI"
    )
    parser.add_argument(
        "-c",
        "--clear",
        action="store_true",
        help="Clear temporary token and login with username and password.",
    )
    args = parser.parse_args()
    if args.clear:
        clear_cached_token()
    print(json.dumps(auth()))
