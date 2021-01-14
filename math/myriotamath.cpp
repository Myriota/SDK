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

Resampler::Resampler(double in_rate, double out_rate, double W)
    : W(W),
      r(myriota_rational_approximation(out_rate / in_rate, 1e-6, 1000, 10)),
      gamma((1.0 * r.p) / r.q),
      kappa(fmin(1, gamma)),
      delta(fmax(1, gamma)),
      xi(r.p > r.q ? r.p : r.q),
      gmin(ceil(-xi * W)),
      gmax(floor(xi * W)),
      a(ceil((2 * W) / kappa + 1), 0.0) {
  // fill resampling filter buffer
  for (int n = gmin; n <= gmax; n++) {
    const double t = (1.0 * n) / xi;
    g_buf.push_back(h(t, W));
  }
}

complex Resampler::operator()(int64_t n) const {
  const double ng = n / gamma;
  const double Wk = W / kappa;
  const int64_t L = ceil(ng - Wk);
  const int64_t U = floor(ng + Wk);
  complex sum = 0;
  for (int64_t m = L; m <= U; m++) sum += a(m) * g(r.q * n - r.p * m);
  return kappa * sum;
}

double Resampler16::g(int64_t n) const { return h((1.0 * n) / xi, W); }

double Resampler16::beta() const {
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

Resampler16::Resampler16(double in_rate, double out_rate, double W)
    : W(W),
      r(myriota_rational_approximation(out_rate / in_rate, 1e-6, 1000, 10)),
      gamma((1.0 * r.p) / r.q),
      kappa(fmin(1, gamma)),
      delta(fmax(1, gamma)),
      xi(r.p > r.q ? r.p : r.q),
      gmin(ceil(-xi * W)),
      gmax(floor(xi * W)),
      alpha(floor((1 << 16) / beta())),
      a(ceil((2 * W) / kappa + 1), (myriota_complex_16){0, 0}) {
  // fill resampling filter buffer
  for (int n = gmin; n <= gmax; n++)
    f_buf.push_back(floor(kappa * alpha * g(n)));
}

myriota_complex_16 Resampler16::operator()(int64_t n) const {
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
  return (myriota_complex_16){myriota_clip_16(re / alpha),
                              myriota_clip_16(im / alpha)};
}

}  // namespace myriota
