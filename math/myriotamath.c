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
#include <assert.h>
#include <stdio.h>

unsigned int myriota_greater_power_of_two(unsigned int x) {
  unsigned int p = 1;
  while (p < x) p *= 2;
  return p;
}

uint64_t myriota_greater_power_of_two_long(uint64_t x) {
  uint64_t p = 1;
  while (p < x) p *= 2;
  return p;
}

unsigned int myriota_get_bit(const unsigned int n, const uint8_t *x) {
  const uint8_t h = x[n / 8];
  const uint8_t r = 7 - (n % 8);
  return (h >> r) & 1;
}

void myriota_set_bit(const unsigned int n, const unsigned int value,
                     uint8_t *x) {
  const unsigned int d = n / 8;
  const uint8_t r = 7 - (n % 8);
  x[d] &= ~(1 << r);  // clear bit
  if (value != 0) x[d] |= (1 << r);
}

void myriota_write_bits(const uint8_t *from, uint8_t *to,
                        const unsigned int start, const unsigned int stop) {
  for (int n = start; n <= stop; n++)
    myriota_set_bit(n, myriota_get_bit(n - start, from), to);
}

void myriota_read_bits(const uint8_t *from, uint8_t *to,
                       const unsigned int start, const unsigned int stop) {
  for (int n = start; n <= stop; n++)
    myriota_set_bit(n - start, myriota_get_bit(n, from), to);
}

myriota_complex myriota_polar(double magnitude, double phase) {
  return magnitude * (cos(phase) + I * sin(phase));
}

myriota_complex myriota_rectangular(double re, double im) {
  return re + im * I;
}

double myriota_complex_norm(myriota_complex x) {
  return creal(x) * creal(x) + cimag(x) * cimag(x);
}

double myriota_complex_abs(myriota_complex x) { return cabs(x); }
double myriota_complex_arg(myriota_complex x) { return carg(x); }
double myriota_complex_real(myriota_complex x) { return creal(x); }
double myriota_complex_imag(myriota_complex x) { return cimag(x); }
myriota_complex myriota_complex_conj(myriota_complex x) { return conj(x); }

double myriota_sinc(double t) {
  if (fabs(t) < 5e-3) return 1.0 - t * t * (1.0 / 6 - 1.0 / 120 * t * t);
  return sin(pi * t) / (pi * t);
}

double myriota_blackman(double t, double W) {
  if (t < -W || t > W) return 0.0;
  const double a0 = 21.0 / 50;
  const double a1 = 1.0 / 2;
  const double a2 = 2.0 / 25;
  return a0 + a1 * cos(pi * t / W) + a2 * cos(2 * pi * t / W);
}

double myriota_sinh(double x) { return (1 - exp(-2 * x)) / (2 * exp(-x)); }

unsigned long myriota_factorial(unsigned int n) {
  unsigned long ret = 1;
  for (unsigned int j = 1; j <= n; j++) ret *= j;
  return ret;
}

double myriota_besselI(int n, double x) {
  double sum = 0, toadd = 1, tol = 1e-5;
  int m = 0;
  while (fabs(toadd) > tol) {
    unsigned long mf = myriota_factorial(m);
    unsigned long nf = myriota_factorial(m + n);
    toadd = pow(x / 2, 2 * m + n) / mf / nf;
    sum += toadd;
    m++;
  }
  return sum;
}

// converts integer in the range 0 - 63 to a base64 character.
// Return -1 if n is out of range
char myriota_number_to_base64(int n) {
  if (n < 0) return -1;
  if (n < 10) return n + '0';
  if (n < 10 + 26) return n - 10 + 'a';
  if (n < 10 + 26 + 26) return n - 10 - 26 + 'A';
  if (n == 62) return '+';
  if (n == 63) return '/';
  return -1;
}

// convert base64 char to number
// Return -1 if character not valid base64
int myriota_base64_to_number(char b) {
  if (b >= '0' && b <= '9') return b - '0';
  if (b >= 'a' && b <= 'z') return 10 + b - 'a';
  if (b >= 'A' && b <= 'Z') return 10 + 26 + b - 'A';
  if (b == '+') return 62;
  if (b == '/') return 63;
  return -1;
}

int myriota_n_base64_to_buf(const char *s, const size_t n, void *buf) {
  if (n % 4 != 0) return -1;  // only multiples of 4 supported
  for (unsigned int i = 0; i < n * 6; i++) {
    const int b = myriota_base64_to_number(s[i / 6]);
    if (b < 0) return -1;
    const uint8_t c = b;
    const int bit = myriota_get_bit((i % 6) + 2, &c);
    myriota_set_bit(i, bit, buf);
  }
  return n;
}

int myriota_base64_to_buf(const char *s, void *buf) {
  return myriota_n_base64_to_buf(s, strlen(s), buf);
}

int myriota_buf_to_base64(const void *buf, const size_t buf_size, char *s) {
  if (buf_size % 3 != 0) return -1;  // only multiples of 3 supported
  uint8_t n = 0;
  int count = 0;
  for (unsigned int i = 0; i < buf_size * 8; i++) {
    const int bit = myriota_get_bit(i, buf);
    myriota_set_bit((i % 6) + 2, bit, &n);
    if ((i % 6) == 5) {
      s[count] = myriota_number_to_base64(n);
      count++;
      n = 0;
    }
  }
  s[count] = '\0';
  return count;
}

static const char *zbase32 = "ybndrfg8ejkmcpqxot1uwisza345h769";

// converts integer in the range 0 - 31 to a base32 character.
// Return -1 if n is out of range
char myriota_number_to_zbase32(int n) {
  if (n >= 0 && n < 32) return zbase32[n];
  return -1;
}

// convert base32 char to number
// Return -1 if character not valid base32
int myriota_zbase32_to_number(char b) {
  for (int i = 0; i < 32; i++)
    if (zbase32[i] == b) return i;
  return -1;
}

int myriota_buf_to_zbase32(const void *buf, const size_t buf_size, char *s) {
  if (buf_size % 5 != 0) return -1;  // only multiples of 5 supported
  uint8_t n = 0;
  int count = 0;
  for (unsigned int i = 0; i < buf_size * 8; i++) {
    const int bit = myriota_get_bit(i, buf);
    myriota_set_bit((i % 5) + 3, bit, &n);
    if ((i % 5) == 4) {
      s[count] = myriota_number_to_zbase32(n);
      count++;
      n = 0;
    }
  }
  s[count] = '\0';
  return count;
}

int myriota_n_zbase32_to_buf(const char *s, const size_t n, void *buf) {
  for (unsigned int i = 0; i < n * 5; i++) {
    const int b = myriota_zbase32_to_number(s[i / 5]);
    if (b < 0) return -1;
    const uint8_t c = b;
    const int bit = myriota_get_bit((i % 5) + 3, &c);
    myriota_set_bit(i, bit, buf);
  }
  return n;
}

int myriota_zbase32_to_buf(const char *s, void *buf) {
  return myriota_n_zbase32_to_buf(s, strlen(s), buf);
}

int myriota_random_bernoulli(double p) {
  double r = myriota_random_uniform();
  if (r < p) return 1;
  return 0;
}

int myriota_random_geometric(double p) {
  assert(p > 0.0);  // p == 0.0 will run forever
  int i = 1;
  while (!myriota_random_bernoulli(p)) i++;
  return i;
}

// Generate a normally distributed random variable.
// Uses the Box Muller method
double myriota_random_normal() {
  const double eps = 1e-15;
  double u = myriota_random_uniform();
  double v = myriota_random_uniform();
  while (u < eps) u = myriota_random_uniform();
  return sqrt(-2 * log(u)) * cos(2 * pi * v);
}

double myriota_random_exponential(const double mean) {
  const double u = myriota_random_uniform();
  return -log(u) * mean;
}

int myriota_random_discrete(const double *p, int n, double u) {
  double sum = 0.0;
  for (int i = 0; i < n; i++) sum += p[i];
  if (sum == 0.0) return (u * n);  // uniform random element
  double cdf = 0;
  for (int i = 0; i < n; i++) {
    cdf += p[i] / sum;
    if (cdf >= u) return i;
  }
  return n - 1;
}

double myriota_continued_fraction(double x, unsigned int size, int *a) {
  int an = (int)floor(x);
  double rem = x - an;
  (*a) = an;
  if (size == 1) return rem;
  return myriota_continued_fraction(1.0 / rem, size - 1, a + 1);
}

long long gcd(long long a, long long b) {
  if (b == 0) return llabs(a);
  return gcd(llabs(b), llabs(a) % llabs(b));
}

myriota_rational make_myriota_rational(long long a, long long b) {
  const int d = gcd(a, b);
  if (b < 0) return (myriota_rational){-a / d, -b / d};
  return (myriota_rational){a / d, b / d};
}

myriota_rational myriota_rational_sum(myriota_rational a, myriota_rational b) {
  return make_myriota_rational(a.p * b.q + a.q * b.p, a.q * b.q);
}

int myriota_rational_compare(myriota_rational a, myriota_rational b) {
  const int x = a.p * b.q;
  const int y = b.p * a.q;
  if (x > y) return 1;
  if (x == y) return 0;
  return -1;
}

static void private_best_approximations(const int *a, int hn1, int hn2, int kn1,
                                        int kn2, unsigned int size,
                                        myriota_rational *r) {
  int hn = (*a) * hn1 + hn2;
  int kn = (*a) * kn1 + kn2;
  *r = make_myriota_rational(hn, kn);
  if (size == 1) return;
  private_best_approximations(a + 1, hn, hn1, kn, kn1, size - 1, r + 1);
}

void myriota_best_approximations(double x, unsigned int size,
                                 myriota_rational *r) {
  int a[size];
  // compute continued fraction expansion of x
  myriota_continued_fraction(x, size, a);
  *r = make_myriota_rational(*a, 1);
  private_best_approximations(a + 1, *a, 1, 1, 0, size - 1, r + 1);
}

myriota_rational myriota_rational_approximation(double x, double tol, int qmax,
                                                unsigned int k) {
  myriota_rational r[k];
  myriota_best_approximations(x, k, r);
  if (fabs(x * r[0].q - r[0].p) < fabs(r[0].q * tol)) return r[0];
  for (int i = 1; i < k; i++) {
    // printf(" %d/%d ", r[i].p, r[i].q);
    if (llabs(r[i].q) > qmax) return r[i - 1];
    if (fabs(x * r[i].q - r[i].p) < fabs(r[i].q * tol)) return r[i];
  }
  return r[k - 1];
}
