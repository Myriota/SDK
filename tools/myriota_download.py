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


import argparse
import os
import sys
import requests
import myriota_auth

__domain = "https://api.myriota.com/v1/release"


def get_download_url(id_token, file_key):
    url = "%s/%s" % (__domain, file_key)
    response = requests.get(url, headers={"Authorization": id_token})
    response.raise_for_status()
    return response.text


def download_file(url, output):
    response = requests.get(url)
    response.raise_for_status()
    open(output, "wb").write(response.content)


def main(argv=None):
    """Implements main CLI entrypoint"""

    parser = argparse.ArgumentParser(description="Download the SDK binaries")
    parser.add_argument(
        "filename", type=str, default=None, help="File to download from the SDK bucket."
    )
    parser.add_argument(
        "-o",
        "--output",
        type=str,
        default=None,
        required=False,
        help="Output to this file instead of the filename in the current directory",
    )
    args = parser.parse_args(argv)

    # If output is not provided, use the original filename
    if args.output is None:
        output = os.path.basename(args.filename)
    else:
        output = args.output

    try:
        token = myriota_auth.auth()["IdToken"]
        s3_url = get_download_url(token, args.filename)
        download_file(s3_url, output)
    # Handle http errors from s3 download
    except requests.exceptions.HTTPError as e:
        if e.response.status_code >= 500:
            print("Server error")
            sys.exit(1)
        elif e.response.status_code >= 400:
            print("File does not exist")
            sys.exit(2)
    except requests.exceptions.RequestException as e:
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
