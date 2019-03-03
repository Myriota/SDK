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


from datetime import datetime
import auth
import requests
import json
import time

_domain = "https://api.myriota.com/v1"

def do_query(idtoken, terminalid, range_from=None, limit=None):
    params = []
    if range_from:
        params.append("from={}".format(range_from))
    if limit:
        params.append("limit={}".format(limit))

    url = "?".join(["%s/data/%s/Message" % (_domain, terminalid), "&".join(params)])
    response = requests.get(url, headers={"Authorization": idtoken})

    if response.status_code != 200:
        raise ValueError(response.text)
    return response.json()["Items"]

def main(argv=None, auth_class=auth.Auth):
    """Implements main CLI entrypoint. Parameterized by auth_class to facilitate testing"""
    import getpass
    import argparse
    import os
    import sys

    parser = argparse.ArgumentParser(description='Command line interface for the Myriota Message Store. Use %s <command> -h for help on a specific command.' % os.path.basename(sys.argv[0]))

    subparsers = parser.add_subparsers(dest='command', title='Valid commands')

    sub_parser = subparsers.add_parser('query', help="Query Message Store for received messages", description="Query Message Store for received messages")
    sub_parser.add_argument("terminalid", help="Terminal Id")
    sub_parser.add_argument('-u', '--user', type=str, default=None,
      help='User. If not provided you will be prompted.')
    sub_parser.add_argument("-f", "--from", dest="range_from", help="Start of date range to query (UTC) e.g. 2018-04-21")
    sub_parser.add_argument("-l", "--limit", help="Maximum number of entries to return")

    args = parser.parse_args(argv)

    if args.command == "query":
        range_from = None
        if args.range_from is not None:
            try:
                dt = datetime.strptime(args.range_from, '%Y-%m-%d')
                range_from = long((dt - datetime.utcfromtimestamp(0)).total_seconds()) * 1000
            except:
                sys.exit("Invalid from. Must be UTC time with format: YYYY-MM-DD")

        limit = None
        if args.limit is not None:
            try:
                limit = int(args.limit)
                assert limit > 0
            except:
                sys.exit("Invalid limit. Must be an integer which is greater than zero")

        authenticator = auth_class()
        authenticator.login(args.user)

        items = do_query(authenticator.id_token, args.terminalid, range_from, args.limit)
        return json.dumps(items, indent=2)
    else:
        return "Invalid command"

if __name__ == "__main__":
    print(main())
