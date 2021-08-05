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

#include "math/myriotamath.h"

namespace myriota {

static double h(double t, double W) {
  return myriota_sinc(t) * myriota_blackman(t, W);
}

ResampleDouble::ResampleDouble(double in_rate, double out_rate, double W)
    : Resample<complex>(in_rate, out_rate, W, 0.0) {
  for (int n = gmin; n <= gmax; n++) {
    const double t = (1.0 * n) / xi;
    g_buf.push_back(h(t, W));
  }
}

complex ResampleDouble::operator()(int64_t n) const {
  const double ng = n / gamma;
  const double Wk = W / kappa;
  const int64_t L = ceil(ng - Wk);
  const int64_t U = floor(ng + Wk);
  complex sum = 0;
  for (int64_t m = L; m <= U; m++) sum += a(m) * g(r.q * n - r.p * m);
  return kappa * sum;
}

double Resample16::g(int64_t n) const { return h((1.0 * n) / xi, W); }

double Resample16::beta() const {
  double b = 0;
  for (int n = 0; n < r.p; n++) {
    const double ng = n / gamma;
    const double Wk = W / kappa;
    const int64_t U = floor(ng + Wk);
    const int64_t L = ceil(ng - Wk);
    double gsum = 0;
    for (int64_t l = L; l <= U; l++) gsum += fabs(g(r.q * n - r.p * l));
    if (gsum > b) b = gsum;
  }
  return kappa * b;
}

Resample16::Resample16(double in_rate, double out_rate, double W)
    : Resample<myriota_complex_16>(in_rate, out_rate, W,
                                   (myriota_complex_16){0, 0}),
      alpha(floor((1 << 16) / beta())) {
  for (int n = gmin; n <= gmax; n++)
    f_buf.push_back(floor(kappa * alpha * g(n)));
}

myriota_complex_32 Resample16::n32(int64_t n) const {
  const double ng = n / gamma;
  const double Wk = W / kappa;
  const int64_t U = floor(ng + Wk);
  const int64_t L = ceil(ng - Wk);
  int32_t re = 0, im = 0;
  int64_t pm = r.q * n - r.p * L;
  for (int64_t m = L; m <= U; m++) {
    const int32_t fv = f(pm);
    pm -= r.p;
    const myriota_complex_16 am = a(m);
    re += am.re * fv;
    im += am.im * fv;
  }
  return (myriota_complex_32){re, im};
}

myriota_complex_16 Resample16::operator()(int64_t n) const {
  const myriota_complex_32 x = n32(n);
  return (myriota_complex_16){myriota_clip_16(x.re / alpha),
                              myriota_clip_16(x.im / alpha)};
}

// signed divide by 2^s witout integer division
static int32_t sdiv(const int32_t x, const int s) {
  if (x >= 0)
    return x >> s;
  else
    return (x + (1 << s)) >> s;
}

myriota_complex_16 Resample16shift::operator()(int64_t n) const {
  const myriota_complex_32 x = n32(n);
  return (myriota_complex_16){myriota_clip_16(sdiv(x.re, s)),
                              myriota_clip_16(sdiv(x.im, s))};
}

}  // namespace myriota
