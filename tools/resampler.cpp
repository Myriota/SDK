// Copyright (c) 2016-2020, Myriota Pty Ltd, All Rights Reserved
// SPDX-License-Identifier: BSD-3-Clause-Attribution
//
// This file is licensed under the BSD with attribution  (the "License"); you
// may not use these files except in compliance with the License.
//
// You may obtain a copy of the License here:
// LICENSE-BSD-3-Clause-Attribution.txt and at
// https://spdx.org/licenses/BSD-3-Clause-Attribution.html
//
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <cstdint>
#include "math/myriotamath.h"
#include "tools/cmdline.h"

using namespace myriota;

// Read complex sample from file
int read_sample(FILE *infile, myriota::complex &sample) {
  double buf[2];
  if (fread(buf, sizeof(double), 2, infile) < 2) return -1;
  sample = myriota::complex(buf[0], buf[1]);
  return 0;
}

// write complex sample to stdout
void write_sample(const myriota::complex &sample) {
  const double buf[2] = {std::real(sample), std::imag(sample)};
  if (fwrite(buf, sizeof(double), 2, stdout) == 2) return;
  fprintf(stderr, "resampler failed to write sample\n");
  exit(EXIT_FAILURE);
}

template <class R>
void resample(FILE *infile, R &r) {
  for (int64_t n = 0; n >= 0; n++) {
    while (r.maxn() < n) {
      complex x;
      if (read_sample(infile, x)) return;
      r.push(x);
    }
    write_sample(r(n));
  }
}

int main(int argc, char **argv) {
  cmdline::parser cmd_parser;

  cmd_parser.add<double>("input_rate", 'i', "input sample rate", true);
  cmd_parser.add<double>("output_rate", 'r', "output sample rate", true);
  cmd_parser.add<double>("window_width", 'W',
                         "larger is slower, but more accurate", false, 30);
  cmd_parser.set_description(
      "Resamples double precision complex samples from input rate to output\n"
      "rate. Input samples via stdin, output samples are written to stdout.\n");

  cmd_parser.parse_check(argc, argv);

  const double in_rate = cmd_parser.get<double>("input_rate");
  const double out_rate = cmd_parser.get<double>("output_rate");
  const double W = cmd_parser.get<double>("window_width");

  /// Upsampler or downsampler as required
  if (in_rate <= out_rate) {
    Upsampler r = Upsampler(in_rate, out_rate, W);
    resample(stdin, r);
  } else {
    Downsampler r = Downsampler(in_rate, out_rate, W);
    resample(stdin, r);
  }

  return EXIT_SUCCESS;
}
