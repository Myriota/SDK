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

import sys
import myriota_auth
import requests
import json

_domain = "https://api.myriota.com/v1"


def do_query(idtoken, moduleid, range_from=None, limit=None):
    params = []
    if range_from:
        params.append("from={}".format(range_from))
    if limit:
        params.append("limit={}".format(limit))

    url = "?".join(["%s/data/%s/Message" % (_domain, moduleid), "&".join(params)])
    response = requests.get(url, headers={"Authorization": idtoken})

    response.raise_for_status()

    return response.json()["Items"]


def main(argv=None, auth=myriota_auth.auth):
    """CLI entrypoint."""
    import argparse
    import os

    parser = argparse.ArgumentParser(
        description="Command line interface for the Myriota Message Store. Use %s <command> -h for help on a specific command."
        % os.path.basename(sys.argv[0])
    )
    subparsers = parser.add_subparsers(dest="command", title="Valid commands")
    sub_parser = subparsers.add_parser(
        "query",
        help="Query Message Store for received messages",
        description="Query Message Store for received messages",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    sub_parser.add_argument("moduleid", help="Module Id")
    sub_parser.add_argument(
        "-f",
        "--from",
        dest="range_from",
        type=int,
        default=0,
        help="Unix epoch second to start query from",
    )
    sub_parser.add_argument(
        "-l",
        "--limit",
        type=int,
        default=100,
        help="Maximum number of entries to return",
    )

    args = parser.parse_args(argv)

    # Validate inputs
    if args.limit <= 0:
        sys.exit("Invalid limit. Must be an integer which is greater than zero")

    if args.command == "query":
        range_from = args.range_from * 1000
        idtoken = auth()["IdToken"]
        try:
            items = do_query(idtoken, args.moduleid, range_from, args.limit)
        except requests.exceptions.RequestException as e:
            raise SystemExit(e)
        return json.dumps(items, indent=2)
    else:
        return "Invalid command"


if __name__ == "__main__":
    print(main())
