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

import base64
import codecs
import datetime
import getpass
import hashlib
import hmac
import os
import re
import requests
import six
import sys
import threading
import time

class ForceChangePasswordException(Exception):
    """Raised when the user is forced to change their password"""



CLIENT_ID = "4jskgo1eq6ngcimlseerg50uvd"
POOL_ID = "us-east-1_XPsvpGiD7"

def _post(endpoint, args, region='us-east-1'):
        headers = {
            "Content-Type": "application/x-amz-json-1.1",
            "X-Amz-Target": "AWSCognitoIdentityProviderService.%s" % endpoint
        }
        url = "https://cognito-idp.%s.amazonaws.com/" % region
        response = requests.post(url, json=args, headers=headers)
        result = response.json()
        if response.status_code != 200:
            raise ValueError("; ".join([result["__type"], str(response.status_code), result["message"]]))

        return result

class AWSPassword(object):
    '''AWS authentication with password'''

    NEW_PASSWORD_REQUIRED_CHALLENGE = 'NEW_PASSWORD_REQUIRED'

    def __init__(self, pool_id, client_id):
        self.pool_id = pool_id
        self.client_id = client_id

    def authenticate_user(self, username, password):
        auth_params = {'USERNAME': username,
                       'PASSWORD': password}

        req = {
            "AuthFlow": "USER_PASSWORD_AUTH",
            "ClientId": self.client_id,
            "AuthParameters" : auth_params
        }

        response = _post('InitiateAuth', req)
        if 'ChallengeName' in response:
            if response['ChallengeName'] == self.NEW_PASSWORD_REQUIRED_CHALLENGE:
                raise ForceChangePasswordException('Please change your password before authenticating.')

            else:
                raise NotImplementedError('The %s challenge is not supported' % response['ChallengeName'])

        return response['AuthenticationResult']

    def set_new_password_challenge(self, username, old_password, new_password):
        auth_params = {'USERNAME': username,
                       'PASSWORD': old_password}

        req = {
            "AuthFlow": "USER_PASSWORD_AUTH",
            "ClientId": self.client_id,
            "AuthParameters" : auth_params
        }

        response = _post('InitiateAuth', req)
        if 'ChallengeName' in response:
            if response['ChallengeName'] == self.NEW_PASSWORD_REQUIRED_CHALLENGE:
                challenge_response = {
                    'USERNAME': auth_params['USERNAME'],
                    'NEW_PASSWORD': new_password
                }
                req = {
                    "ChallengeName": "NEW_PASSWORD_REQUIRED",
                    "ClientId": self.client_id,
                    "Session": response["Session"],
                    "ChallengeResponses": challenge_response
                }
                new_password_response = _post('RespondToAuthChallenge', req)
                return new_password_response["AuthenticationResult"]
            else:
                raise NotImplementedError('The %s challenge is not supported' % response['ChallengeName'])

        return response['AuthenticationResult']

    def refresh_tokens(self, refresh_token):
        req = {
            "AuthFlow": "REFRESH_TOKEN_AUTH",
            "ClientId": self.client_id,
            "AuthParameters" : {
                'REFRESH_TOKEN': refresh_token
            }
        }

        return _post('InitiateAuth', req)

class Auth(object):
    '''User-facing wrapper class for AWSPassword.'''

    def __init__(self, aws_auth=None):
        '''
        The aws_auth and the functions for username and password prompt are
        customisable.
        '''
        self.aws_auth = aws_auth or AWSPassword(client_id=CLIENT_ID, pool_id=POOL_ID)
        self.refresh_before_expiry = 300
        self.tokens = {'IdToken': None, 'AccessToken': None, 'RefreshToken': None, 'ExpiresIn': None}

    def username(self, prompt="User: "):
        sys.stderr.write(prompt)
        try:
            # Try python2 first
            return raw_input()
        except NameError:
            return input()

    def password(self, prompt="Password: "):
        return getpass.getpass(prompt)

    def refresh(self, refresh_token):
        return self.aws_auth.refresh_tokens(refresh_token)['AuthenticationResult']

    def login(self, username=None, password=None, new_password=None, auto_refresh=False):
        '''
        Performs login and returns authentication tokens. Prompts for username and
        password if necessary.
        '''
        username = username or self.username()

        attempts = 0
        while True:
            password = password or self.password()
            try:
                self.tokens = self.aws_auth.authenticate_user(username, str(password))
                break
            except ForceChangePasswordException:
                if new_password is None:
                    print('Please reset your password', file=sys.stderr)
                    new_password = self.password("New password: ")
                self.tokens =self.aws_auth.set_new_password_challenge(username, str(password), str(new_password))
                break
            except Exception as e:
                if attempts < 2:
                    attempts += 1
                    password = None
                    print('Login failed ' + str(e), file=sys.stderr)
                    continue
                else:
                    print('Login failed ' + str(e), file=sys.stderr)
                    raise ValueError('Maximum number of attempts exceeded.')

        if auto_refresh:
            self.start_refresh()

    def start_refresh(self):
        # this is busted out of the main function to ease testing
        self.tokens_expiry = time.time() + self.tokens['ExpiresIn']
        refresh_interval = self.tokens['ExpiresIn'] - self.refresh_before_expiry
        self.refresh_timer = threading.Timer(refresh_interval, self.refresh_tokens)
        self.refresh_timer.daemon = True
        self.refresh_timer.start()

    def refresh_tokens(self, refresh_token=None, auto_refresh=True):
        '''Periodic refresh of authentication tokens.'''

        if refresh_token is None:
            refresh_token = self.tokens['RefreshToken']
        else:
            # store the refresh token for next time around
            self.tokens['RefreshToken'] = refresh_token

        if refresh_token is None:
            raise ValueError('Authentication error: no refresh token available.')

        try:
            refresh_tokens = self.refresh(refresh_token)
        except Exception as e:
            # refresh not successful -> retry in 30 seconds
            refresh_interval = 30
            if not hasattr(self, "tokens_expiry") or time.time() > self.tokens_expiry:
                raise ValueError('Authentication error: unable to refresh tokens.')
        else:
            # refresh successful -> update tokens and set new expiry time
            self.tokens.update(refresh_tokens)
            self.tokens_expiry = time.time() + self.tokens['ExpiresIn']
            refresh_interval = self.tokens['ExpiresIn'] - self.refresh_before_expiry

        if auto_refresh:
            self.refresh_timer = threading.Timer(refresh_interval, self.refresh_tokens)
            self.refresh_timer.daemon = True
            self.refresh_timer.start()

    @property
    def id_token(self):
        return self.tokens['IdToken']

    @property
    def access_token(self):
        return self.tokens['AccessToken']

    @property
    def refresh_token(self):
        return self.tokens['RefreshToken']

if __name__ == '__main__':
    import json
    import argparse

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command')

    parser_a = subparsers.add_parser('login')
    parser_a.add_argument('-u', '--user', type=str, default=None,
      help='User. If not provided you will be prompted.')
    parser_a.add_argument('-r', '--request_token', action="store_true",
      help='Print the refresh token to stdout.')

    parser_b = subparsers.add_parser('refresh')
    parser_b.add_argument("refresh_token", help="Cognito refresh token")

    args = parser.parse_args()

    authenticator = Auth()

    try:
        if args.command == "login":

            authenticator.login(args.user)
            if args.request_token:
                print(authenticator.refresh_token)
            else:
                print(json.dumps(authenticator.tokens, indent=2))

        elif args.command == "refresh":

            tokens = authenticator.refresh(args.refresh_token)
            print(json.dumps(tokens, indent=2))

    except KeyboardInterrupt as e:
        sys.exit(e)
    except Exception as e:
        sys.exit(e)
