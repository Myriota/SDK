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

#ifndef CONVERT_TYPE_H
#define CONVERT_TYPE_H

#include <stdio.h>
#include <complex>
#include <limits>
#include <string>

typedef std::complex<double> complex;

// support signed and unsigned types
template <typename T>
const double offset() {
  const double lowest = std::numeric_limits<T>::lowest();
  const double highest = std::numeric_limits<T>::max();
  if (lowest < 0) return 0.0;
  return ceil((highest - lowest) / 2.0);
}

// read complex samples of templated type from input file
template <typename T>
bool read_sample_of_type(FILE *infile, complex &sample) {
  const double off = offset<T>();
  T buf[2];
  if (fread(buf, sizeof(T), 2, infile) < 2) return false;
  sample = complex(buf[0] - off, buf[1] - off);
  return true;
}

// saturation
template <typename T>
double limit(double x, bool &clipped) {
  const double lowest = std::numeric_limits<T>::lowest();
  const double highest = std::numeric_limits<T>::max();
  clipped = false;
  if (x > highest) {
    x = highest;
    clipped = true;
  }
  if (x < lowest) {
    x = lowest;
    clipped = true;
  }
  return x;
}

// read sample from line, complex and imaginary parts tab seperated.
bool read_sample_txt(FILE *infile, complex &sample) {
  float r, i;
  if (fscanf(infile, "%f\t%f%*[^\n]\n", &r, &i) != 2) return false;
  sample = complex(r, i);
  return true;
}

// print sample to a line in txt format, complex and imaginary parts tab
// seperated.
bool print_sample_txt(FILE *file, const complex sample) {
  printf("%f\t%f\n", std::real(sample), std::imag(sample));
  return false;  // never clip
}

// cast complex sample to templated type and print sample to stdout
template <typename T>
bool print_sample_of_type(FILE *file, const complex sample) {
  const double off = offset<T>();
  bool clipped;
  const T sample_2[2] = {
      static_cast<T>(limit<T>(std::real(sample) + off, clipped)),
      static_cast<T>(limit<T>(std::imag(sample) + off, clipped))};
  fwrite(sample_2, sizeof(T), 2, file);
  return clipped;
}

#endif
