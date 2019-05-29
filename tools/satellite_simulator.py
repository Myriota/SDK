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

import bz2
from distutils.spawn import find_executable
import getpass
from io import BytesIO
import os.path
import requests
import subprocess
import sys
import time
from threading import Timer, Thread
from uuid import uuid4


class SatelliteSimulator(object):
    '''
    Myriota Development Kit satellite simulator. Captures transmissions from
    Myriota Development Boards with a satellite simulator USB dongle and uploads
    the signals to the Myriota Cloud.
    '''

    def __init__(
        self, frequency=434e6, rate=250e3, down_rate=5e3, gain=33.8,
        chunk_duration=60,
        api_url='https://api.myriota.com/v1/spectrum/ingest/',
        id=None, url_token=lambda: None
    ):
        # endpoint and token for capture upload
        self.api_url = api_url.strip('/')
        self.url_token = url_token

        # use fixed simulator ID if provided or generate a UUID
        self.id = id if (id is not None and id != '') else str(uuid4())

        # radio parameters
        self.frequency = frequency
        self.rate = rate
        self.down_rate = down_rate
        self.gain = gain

        # chunks with int16 IQ samples
        self.chunk_duration = chunk_duration
        self.chunk_size = int(self.chunk_duration * self.down_rate * 4)

        # tools convert_type and resampler assumed to reside in current folder
        os.environ["PATH"] += os.pathsep + os.getcwd()

    def check_dongle(self):
        '''
        Check if rtl_sdr, convert_type and resampler tools are installed and
        device available. No need to reimplement dongle search as rtl_sdr
        already does this. Returns True on success.
        '''
        if find_executable('rtl_sdr') is None:
            raise IOError('rtl_sdr: command not found. Please check your installation.')
        if find_executable('convert_type') is None:
            raise IOError('convert_type: command not found. Please check your installation.')
        if find_executable('resampler') is None:
            raise IOError('resampler: command not found. Please check your installation.')
        proc = subprocess.Popen('rtl_sdr -n 1 - > /dev/null', shell=True, stderr=subprocess.PIPE)
        _, stderr = proc.communicate()
        if proc.returncode != 0:
            raise IOError(stderr.strip())
        return True

    def start_capture(self, duration=0):
        '''
        Initiate signal capture and processing chain. Samples are written to
        stdout attribute of the returned subprocess.Popen object. Units of the
        duration is seconds, with zero corresponding to capturing indefinitely.
        '''
        cmd = [
            'rtl_sdr -f {frequency} -s {rate} -g {gain} -n {samples} -',
            'convert_type -f uint8',
            'resampler -i {rate} -r {down_rate}',
            'convert_type -t int16'
        ]
        cmd = ' | '.join(cmd).format(
            frequency=self.frequency,
            rate=self.rate,
            gain=self.gain,
            down_rate=self.down_rate,
            samples=int(duration * self.rate)
        )
        return subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)

    def process_capture(self, capture):
        '''
        Reads samples from stdout of the provided subprocess.Popen object
        capture and splits the stream into chunks. Internally invokes the
        function process_chunk for each chunk.

        This function does not return until termination of the capture process.
        '''
        chunk = BytesIO()
        threads = []
        while True:
            returncode = capture.poll()
            if returncode == None:
                buffer = capture.stdout.read(64)
                while len(buffer) >= self.chunk_size - chunk.tell():
                    bytes = self.chunk_size - chunk.tell()
                    chunk.write(buffer[0:bytes])
                    t = Thread(target=self.process_chunk, args=[chunk])
                    threads.append(t)
                    t.start()
                    # remove written portion from buffer, then move on to next chunk
                    buffer = buffer[bytes:]
                    chunk = BytesIO()
                chunk.write(buffer)
            elif returncode == 0:
                # capture process completed successfully -> process remaining samples
                buffer = capture.stdout.read()
                while len(buffer) >= self.chunk_size - chunk.tell():
                    bytes = self.chunk_size - chunk.tell()
                    chunk.write(buffer[0:bytes])
                    t = Thread(target=self.process_chunk, args=[chunk])
                    threads.append(t)
                    t.start()
                    # remove written portion from buffer, then move on to next chunk
                    buffer = buffer[bytes:]
                    chunk = BytesIO()
                chunk.write(buffer)
                if chunk.tell() > 0:
                    t = Thread(target=self.process_chunk, args=[chunk])
                    threads.append(t)
                    t.start()
                break
            else:
                raise IOError('Error: signal capture terminated with exit code {}.'.format(returncode))
            # clear out finished threads
            threads = [t for t in threads if t.is_alive()]
        # wait for remaining threads to finished
        [t.join() for t in threads]

    def process_chunk(self, chunk):
        '''
        Compresses a chunk of samples with bzip2. Internally invokes function
        upload_chunk.
        '''
        chunk.seek(0)
        compressed = bz2.compress(chunk.read(), 9)
        chunk.seek(0)
        chunk.write(compressed)
        chunk.truncate()
        try:
            self.upload_chunk(chunk)
        except Exception as e:
            # tell the user if upload fails, but attempt to keep going
            print('Upload failed:' + str(e), file=sys.stderr)
        finally:
            chunk.close()

    def upload_chunk(self, chunk):
        '''
        Uploads the provided chunk of samples to a presigned URL.
        '''
        upload_url = self.upload_url()
        chunk.seek(0)
        headers = {'Content-Type': ''}
        upload = requests.put(upload_url, headers=headers, data=chunk)
        upload.raise_for_status()

    def get_time(self):
        return time.time()

    def upload_url(self):
        '''
        Returns a presigned URL for uploading a chunk of samples.
        '''
        # timestamp with 100 µs resolution, backdated by the duration of the chunk
        timestamp = int((self.get_time() - self.chunk_duration) * 1e4)
        acquisition_id = '{id}_{ts}.bz2'.format(id=self.id, ts=timestamp)
        auth_header = {'Authorization': '{token}'.format(token=self.url_token())}
        get_params = {
            'AcqID': acquisition_id,
            'sample-rate': str(self.down_rate)
        }
        destination = requests.get(self.api_url, headers=auth_header, params=get_params)
        destination.raise_for_status()
        return destination.text


class SatelliteSimulatorAuth(SatelliteSimulator):
    '''Myriota Development Kit satellite simulator with user authentication.'''

    def __init__(self, **kwargs):
        # authentication
        from auth import Auth

        aws_auth = kwargs.pop('aws_auth', None)
        self.auth = Auth(aws_auth=aws_auth)

        # Instantiate base class with a function to dynamically update auth token
        SatelliteSimulator.__init__(self, url_token=lambda: self.auth.id_token, **kwargs)

    def login(self, username=None, password=None, auto_refresh=True):
        '''
        Prompt for username and password if needed, then log in, and schedule
        periodic refresh of authentication tokens. Returns True on success.
        '''
        self.auth.login(username, password, auto_refresh=auto_refresh)
        return True

    def refresh_tokens(self, refresh_token, auto_refresh=True):
        '''
        Authenticate using previously obtained refresh token, and schedule
        periodic refresh of authentication tokens. Returns True on success.
        '''
        self.auth.refresh_tokens(refresh_token, auto_refresh)
        return True

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description=SatelliteSimulator.__doc__)
    parser.add_argument('-d', '--duration', type=float, default=0,
      help='Duration of the simulation in seconds. Runs indefinitely if set to zero (default).')
    args = parser.parse_args()

    simulator = SatelliteSimulatorAuth()
    try:
        simulator.check_dongle()
        simulator.login()
        capture = simulator.start_capture(duration=args.duration)
        simulator.process_capture(capture)
    except KeyboardInterrupt as e:
        sys.exit(e)
    except Exception as e:
        sys.exit(e)
