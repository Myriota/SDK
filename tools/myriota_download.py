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


import argparse
import os

import requests

import auth

_domain = "https://api.myriota.com/v1/release"

help_description = """
Script which will download the latest SDK
"""

def authenticate(user):

    authenticator = auth.Auth()
    authenticator.login(user)

    return authenticator.id_token

def get_download_url(id_token, file_key):

    url = "%s/%s" % (_domain, file_key)
    response = requests.get(url, headers={"Authorization": id_token})

    if response.status_code != 200:
        raise ValueError(response.text)
    return response.text

def download_file(url, output):

    response = requests.get(url)
    open(output, 'wb').write(response.content)

def main(argv=None):
    """Implements main CLI entrypoint"""

    parser = argparse.ArgumentParser(description=help_description)

    parser.add_argument(
        'filename', type=str, default=None,
        help='File to download from the SDK bucket.'
    )
    parser.add_argument(
        '-o', '--output', type=str, default=None, required=False,
        help='Output to this file instead of the filename in the current directory'
    )
    parser.add_argument(
        '-u', '--user', type=str, default=None,
        help='User. If not provided you will be prompted.'
    )

    args = parser.parse_args(argv)

    token = authenticate(args.user)

    s3_url = get_download_url(token, args.filename)

    # If output is not provided, use the original filename
    if args.output is None:
        output = os.path.basename(args.filename)
    else:
        output = args.output
    download_file(s3_url, output)

if __name__ == "__main__":
    main()
