// Copyright (c) 2016-2019, Myriota Pty Ltd, All Rights Reserved
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

#include "tools/convert_type.h"
#include <inttypes.h>
#include <stdlib.h>
#include "tools/cmdline.h"

int main(int argc, char **argv) {
  cmdline::parser cmd_parser;

  cmd_parser.set_description(
      "Converts samples from specified input type to specified output type. "
      "This tool performs a straight conversion without rescaling or rounding. "
      "Input samples via stdin, output samples are written to stdout.\n");
  cmd_parser.add<std::string>("from", 'f',
                              "input data type: double, float, uint8, "
                              "int8, int16, uint16, int32",
                              false, "double");
  cmd_parser.add<std::string>("to", 't',
                              "output data type: double, float, uint8, "
                              "int8, int16, uint16, int32",
                              false, "double");
  cmd_parser.add("exit-on-clip", 'e',
                 "program will exit with EXIT_FAILURE return code if any "
                 "clipping detected");
  cmd_parser.parse_check(argc, argv);

  // Get input/output type and check if conversion necessary
  const std::string input_type = cmd_parser.get<std::string>("from");
  const std::string output_type = cmd_parser.get<std::string>("to");
  const bool exit_on_clip = cmd_parser.exist("exit-on-clip");

  // assign function to read samples from file
  bool (*read_sample)(FILE * stdin, complex & sample);
  if (input_type == "double")
    read_sample = read_sample_of_type<double>;
  else if (input_type == "float")
    read_sample = read_sample_of_type<float>;
  else if (input_type == "uint8")
    read_sample = read_sample_of_type<uint8_t>;
  else if (input_type == "int8")
    read_sample = read_sample_of_type<int8_t>;
  else if (input_type == "int16")
    read_sample = read_sample_of_type<int16_t>;
  else if (input_type == "uint16")
    read_sample = read_sample_of_type<uint16_t>;
  else if (input_type == "int32")
    read_sample = read_sample_of_type<int32_t>;
  else {
    std::cerr << "Input type must be one of double, float, uint8, int8, int16, "
                 "uint16, or int32"
              << std::endl;
    std::cerr << cmd_parser.usage();
    return EXIT_FAILURE;
  }

  // assign function to write sample to stdout
  bool (*print_sample)(FILE *, const complex);
  if (output_type == "double")
    print_sample = print_sample_of_type<double>;
  else if (output_type == "float")
    print_sample = print_sample_of_type<float>;
  else if (output_type == "uint8")
    print_sample = print_sample_of_type<uint8_t>;
  else if (output_type == "int8")
    print_sample = print_sample_of_type<int8_t>;
  else if (output_type == "int16")
    print_sample = print_sample_of_type<int16_t>;
  else if (output_type == "uint16")
    print_sample = print_sample_of_type<uint16_t>;
  else if (output_type == "int32")
    print_sample = print_sample_of_type<int32_t>;
  else {
    std::cerr
        << "Output type must be one of double, float, int8, int16, uint16, or "
           "int32"
        << std::endl;
    std::cerr << cmd_parser.usage();
    return EXIT_FAILURE;
  }

  // the actual conversion loop
  complex sample;
  while (read_sample(stdin, sample)) {
    const bool clipped = print_sample(stdout, sample);
    if (exit_on_clip && clipped) return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
