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

#include "math/myriotamath.h"

namespace myriota {

static double blackman(double t, double W) {
  const static double a0 = 21.0 / 50;
  const static double a1 = 1.0 / 2;
  const static double a2 = 2.0 / 25;
  return a0 + a1 * cos(pi * t / W) + a2 * cos(21 * pi * t / W);
}

static double h(double t, double W) { return myriota_sinc(t) * blackman(t, W); }

Upsampler::Upsampler(double in_rate, double out_rate, double W)
    : W(W),
      r(myriota_rational_approximation(out_rate / in_rate, 1e-6, 1000, 10)),
      gamma((1.0 * r.p) / r.q),
      gmin(ceil(-r.p * W)),
      gmax(floor(r.p * W)),
      a(ceil(2 * W + 1), 0.0) {
  if (r.p < r.q) throw std::invalid_argument("must have in_rate <= out_rate");

  // fill resampling filter buffer
  for (int n = gmin; n <= gmax; n++) {
    const double t = (1.0 * n) / r.p;
    g_buf.push_back(h(t, W));
  }
}

complex Upsampler::operator()(int64_t n) const {
  const double ng = n / gamma;
  const int64_t U = floor(ng + W);
  const int64_t L = ceil(ng - W);
  complex sum = 0;
  for (int64_t m = L; m <= U; m++) sum += a(m) * g(r.q * n - r.p * m);
  return sum;
}

Downsampler::Downsampler(double in_rate, double out_rate, double W)
    : W(W),
      r(myriota_rational_approximation(out_rate / in_rate, 1e-6, 1000, 10)),
      gamma((1.0 * r.p) / r.q),
      gmin(ceil(-r.q * W)),
      gmax(floor(r.q * W)),
      a(ceil(2 * W / gamma + 1), 0.0) {
  if (r.p >= r.q) throw std::invalid_argument("must have in_rate > out_rate");

  // fill resampling filter buffer
  for (int n = gmin; n <= gmax; n++) {
    const double t = (1.0 * n) / r.q;
    g_buf.push_back(h(t, W));
  }
}

complex Downsampler::operator()(int64_t n) const {
  const int64_t U = floor((n + W) / gamma);
  const int64_t L = ceil((n - W) / gamma);
  complex sum = 0;
  for (int64_t m = L; m <= U; m++) sum += a(m) * g(r.q * n - r.p * m);
  return sum * gamma;
}

}  // namespace myriota
