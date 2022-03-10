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

template <class R, class C>
void resample(FILE *infile, R &r) {
  for (int64_t n = 0; n >= 0; n++) {
    while (r.maxn() < n) {
      C x;
      if (fread(&x, sizeof(C), 1, infile) != 1) return;
      r.push(x);
    }
    const C rn = r(n);
    fwrite(&rn, sizeof(C), 1, stdout);
  }
}

int main(int argc, char **argv) {
  cmdline::parser cmd_parser;

  cmd_parser.add<double>("input_rate", 'i', "input sample rate", true);
  cmd_parser.add<double>("output_rate", 'r', "output sample rate", true);
  cmd_parser.add<double>("window_width", 'W',
                         "larger is slower, but more accurate", false, 30);
  cmd_parser.add(
      "int16", '\0',
      "16-bit fixed point implementation, int16 input, int16 output.");
  cmd_parser.add("shift", '\0',
                 "Replace division with shift, only with --int16 option.");
  cmd_parser.add<std::string>("taps", '\0',
                              "print filter taps in verilog format.", false);
  cmd_parser.set_description(
      "Resamples complex samples from input rate to output rate. Input samples "
      "via stdin, output samples are written to stdout. By default the input "
      "samples are expected to be in double precision floating point "
      "format.\n");

  cmd_parser.parse_check(argc, argv);

  const double in_rate = cmd_parser.get<double>("input_rate");
  const double out_rate = cmd_parser.get<double>("output_rate");
  const double W = cmd_parser.get<double>("window_width");

  if (cmd_parser.exist("taps")) {
    Resample16shift r = Resample16shift(in_rate, out_rate, W);
    const char *name = cmd_parser.get<std::string>("taps").c_str();
    printf("reg signed [15:0] %s [0:%d];\n", name, (int)(r.taps().size() - 1));
    printf("initial begin\n");
    for (unsigned int i = 0; i < r.taps().size(); i++)
      printf("%s[%d] = %d;\n", name, i, r.taps()[i]);
    printf("end\n");
    return EXIT_SUCCESS;
  }

  if (cmd_parser.exist("int16") && cmd_parser.exist("shift")) {
    Resample16shift r = Resample16shift(in_rate, out_rate, W);
    resample<Resample16shift, myriota_complex_16>(stdin, r);
  } else if (cmd_parser.exist("int16")) {
    Resample16 r = Resample16(in_rate, out_rate, W);
    resample<Resample16, myriota_complex_16>(stdin, r);
  } else {  // double format by default
    ResampleDouble r = ResampleDouble(in_rate, out_rate, W);
    resample<ResampleDouble, complex>(stdin, r);
  }

  return EXIT_SUCCESS;
}
