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
#include <stdio.h>

double myriota_modulus(const double arg1, const double arg2) {
  const int i = arg1 / arg2;
  const double ret_val = arg1 - i * arg2;
  if (ret_val < 0.0) return ret_val + arg2;
  return ret_val;
}

// returns maximum element of a vector of integers.
// returns INT_MIN if the vector is empty
int myriota_int_array_max(const int *S, unsigned int numS) {
  int cmax = INT_MIN;
  for (int i = 0; i < numS; i++) cmax = myriota_int_max(cmax, S[i]);
  return cmax;
}

// returns the index of the maximum element of a vector of integers.
// returns INT_MIN if the vector is empty
int myriota_int_array_arg_max(const int *S, unsigned int numS) {
  if (numS == 0) return INT_MIN;
  int arg_max = 0;
  for (int i = 1; i < numS; i++)
    if (S[i] > S[arg_max]) arg_max = i;
  return arg_max;
}

// returns minimum element of a vector of integers.
// returns INT_MAX if the vector is empty
int myriota_int_array_min(const int *S, unsigned int numS) {
  int cmin = INT_MAX;
  for (int i = 0; i < numS; i++) cmin = myriota_int_min(S[i], cmin);
  return cmin;
}

// returns the index of the minimum element of a vector of integers.
// returns INT_MAX if the vector is empty
int myriota_int_array_arg_min(const int *S, unsigned int numS) {
  if (numS == 0) return INT_MAX;
  int arg_min = 0;
  for (int i = 1; i < numS; i++)
    if (S[i] < S[arg_min]) arg_min = i;
  return arg_min;
}

bool myriota_is_strictly_ascending(const int *a, const int size) {
  for (int i = 1; i < size; i++)
    if (a[i - 1] >= a[i]) return false;
  return true;
}

int myriota_int_mod(int x, int y) {
  int t = x % y;
  if (t < 0) return t + y;
  return t;
}

long myriota_long_mod(long x, long y) {
  long t = x % y;
  if (t < 0) return t + y;
  return t;
}

int64_t myriota_int64_mod(int64_t x, int64_t y) {
  int64_t t = x % y;
  if (t < 0) return t + y;
  return t;
}

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

myriota_complex myriota_polar(myriota_decimal magnitude,
                              myriota_decimal phase) {
  return magnitude * (cos(phase) + I * sin(phase));
}

myriota_complex myriota_rectangular(myriota_decimal re, myriota_decimal im) {
  return re + im * I;
}

myriota_decimal myriota_complex_norm(myriota_complex x) {
  return creal(x) * creal(x) + cimag(x) * cimag(x);
}

double myriota_sinc(double t) {
  if (fabs(t) < 5e-3)
    return 1.0 - t * t * (1.0 / 6 - 1.0 / 120 * t * t);
  else
    return sin(pi * t) / (pi * t);
}

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

// Convert hexidecimal character to number in the interval [0,15]
// Returns 1 (the number of characters scanned) on success and 0 if the
// character is not hexidecimal
int myriota_hex_character_to_number(const char h, int *n) {
  if (h >= '0' && h <= '9') {
    *n = h - '0';
    return 1;
  }
  if (h >= 'A' && h <= 'F') {
    *n = h - 'A' + 10;
    return 1;
  }
  if (h >= 'a' && h <= 'f') {
    *n = h - 'a' + 10;
    return 1;
  }
  return 0;
}

bool myriota_is_hex(const char *s) {
  int n;
  for (int i = 0; i < strlen(s); i++)
    if (myriota_hex_character_to_number(s[i], &n) == 0) return false;
  return true;
}

int myriota_hex_to_byte(const char *h, uint8_t *b) {
  int h0;
  if (myriota_hex_character_to_number(h[0], &h0) != 1) return 0;
  int h1;
  if (myriota_hex_character_to_number(h[1], &h1) != 1) return 0;
  *b = 0;
  *b = *b | h1;
  *b = *b | (h0 << 4);
  return 2;
}

int myriota_n_hex_to_buf(const char *s, const size_t n, void *buf) {
  uint8_t *bytes = (uint8_t *)buf;
  const size_t slen = strlen(s);
  const size_t m = n > slen ? slen : n;
  if (m % 2 != 0) return 0;  // n must be even
  for (const char *c = s; c < s + m; c += 2, bytes++)
    if (myriota_hex_to_byte(c, bytes) != 2) return 0;  // scan failed
  return m;                                            // success
}

int myriota_hex_to_buf(const char *s, void *buf) {
  return myriota_n_hex_to_buf(s, strlen(s), buf);
}

int myriota_buf_to_hex(const void *buf, const size_t buf_size, char *s) {
  const uint8_t *bytes = (const uint8_t *)buf;
  for (size_t i = 0; i < buf_size; i++) s += sprintf(s, "%02x", bytes[i]);
  return buf_size * 2;
}

void myriota_print_hex(const void *buf, int length) {
  const uint8_t *b = (const uint8_t *)buf;
  for (int i = 0; i < length; i++) printf("%02x", b[i]);
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

// Reverses bits in a 32-bit word.
static uint32_t reverse_bits(uint32_t x) {
  x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
  x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
  x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
  x = (x << 24) | ((x & 0xFF00) << 8) | ((x >> 8) & 0xFF00) | (x >> 24);
  return x;
}

uint32_t myriota_crc32(const void *data, size_t length, uint32_t offset) {
  uint32_t crc = ~offset;
  const uint8_t *bytes = (uint8_t *)data;
  for (int i = 0; i < length; i++) {
    uint32_t b = reverse_bits(bytes[i]);
    for (int j = 0; j < 8; j++) {
      if ((int32_t)(crc ^ b) < 0)
        crc = (crc << 1) ^ 0x04C11DB7;
      else
        crc = crc << 1;
      b = b << 1;
    }
  }
  return reverse_bits(~crc);
}

int myriota_random_bernoulli(double p) {
  double r = myriota_random_uniform();
  if (r < p) return 1;
  return 0;
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

double myriota_continued_fraction(double x, unsigned int size, int *a) {
  int an = (int)floor(x);
  double rem = x - an;
  (*a) = an;
  if (size == 1) return rem;
  return myriota_continued_fraction(1.0 / rem, size - 1, a + 1);
}

long long gcd(long long a, long long b) {
  if (b == 0)
    return abs(a);
  else
    return gcd(abs(b), abs(a) % abs(b));
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
    if (abs(r[i].q) > qmax)
      return r[i - 1];
    else if (fabs(x * r[i].q - r[i].p) < fabs(r[i].q * tol))
      return r[i];
  }
  return r[k - 1];
}

double myriota_bisection(double (*f)(double, void *), void *fdata, double ax,
                         double bx, double tol, unsigned int max_iterations) {
  double a = ax;
  double b = bx;
  double xzero = 0;
  for (unsigned int i = 1; i <= max_iterations; i++) {
    double c = (a + b) / 2;
    double fc = f(c, fdata);
    if (fc == 0 || fabs(a - b) / 2 < tol) {
      xzero = c;
      return xzero;
    }
    if (myriota_signum(fc) == myriota_signum(f(a, fdata)))
      a = c;
    else
      b = c;
  }
  return xzero;
}

double myriota_bisection_default(double (*f)(double, void *), void *fdata,
                                 double a, double b) {
  return myriota_bisection(f, fdata, a, b, 1e-5, 100);
}

// contains function to solve for and desired value
struct solve_struct {
  double (*f)(double, void *);
  void *data;
  const double y;
};

// offset function, i.e. solve_offset(x) = f(x) - y
// make this function suitble for the bisection method
static double solve_offset(double x, void *data) {
  struct solve_struct *s = data;
  return s->f(x, s->data) - s->y;
}

double myriota_solve(double (*f)(double, void *), void *fdata, double y,
                     double ax, double bx, double tol,
                     unsigned int max_iterations) {
  struct solve_struct s = {f, fdata, y};
  return myriota_bisection(solve_offset, &s, ax, bx, tol, max_iterations);
}

double myriota_unwrap(const double value, const double previous_value) {
  double d = myriota_fracpart_scaled(value - previous_value, 2 * pi);
  if (d > pi)
    return d + previous_value - 2 * pi;
  else if (d < -pi)
    return d + previous_value + 2 * pi;
  else
    return d + previous_value;
}

static int cmp_double(const void *ap, const void *bp) {
  double a = *(double *)ap;
  double b = *(double *)bp;
  if (a > b)
    return 1;
  else if (a < b)
    return -1;
  else
    return 0;
}

double myriota_select_double(const int k, double *a, size_t nitems) {
  qsort(a, nitems, sizeof(double), cmp_double);
  return a[k];
}

static int cmp_int32(const void *a, const void *b) {
  return (*(int32_t *)a - *(int32_t *)b);
}

int32_t myriota_select_int32(const int k, int32_t *a, size_t nitems) {
  qsort(a, nitems, sizeof(int32_t), cmp_int32);
  return a[k];
}

double myriota_median_double(double *a, const size_t nitems) {
  if (nitems % 2 != 0)
    return myriota_select_double(nitems / 2, a, nitems);
  else {
    const double upper = myriota_select_double(nitems / 2, a, nitems);
    const double lower = myriota_select_double(nitems / 2 - 1, a, nitems);
    return (lower + upper) / 2.0;
  }
}

int32_t myriota_median_int32(int32_t *a, const size_t nitems) {
  if (nitems % 2 != 0)
    return myriota_select_int32(nitems / 2, a, nitems);
  else {
    const int64_t upper = myriota_select_int32(nitems / 2, a, nitems);
    const int64_t lower = myriota_select_int32(nitems / 2 - 1, a, nitems);
    return (lower + upper) / 2;
  }
}

// function used internally by myriota_brent
double myriota_brent_sign(double a, double b) {
  return fabs(a) * myriota_signum(b);
}

unsigned int myriota_brent(double (*f)(double, void *), void *fdata,
                           const double ax, const double bx, const double cx,
                           double *fx, double *x, const double tol,
                           const unsigned int max_iterations) {
  // close to machine precision (works for both float and double definitions
  // of double)
  const double eps = sizeof(double) == 8 ? 1e-10 : 1e-6;
  const double C = (3.0 - sqrt(5.0)) / 2.0;

  double e = 0.0;
  double d = 0.0;
  double a = (ax < cx ? ax : cx);
  double b = (ax > cx ? ax : cx);
  *x = bx;
  double w = bx;
  double v = bx;
  double fw = f(*x, fdata);
  double fv = fw;
  *fx = fw;
  for (unsigned int iter = 0; iter < max_iterations; iter++) {
    const double xm = 0.5 * (a + b);
    const double tol1 = tol + eps;
    const double tol2 = 2.0 * tol1;
    if (fabs(*x - xm) <= tol2 - 0.5 * (b - a)) return iter;  //(fx, x)

    if (fabs(e) > tol1) {
      const double r = (*x - w) * (*fx - fv);
      double q = (*x - v) * (*fx - fv);
      double p = (*x - v) * q - (*x - v) * r;
      q = 2.0 * (q - r);
      if (q > 0.0) p = -p;
      q = fabs(q);
      const double etemp = e;
      e = d;
      if (fabs(p) >= fabs(0.5 * q * etemp) || p <= q * (a - *x) ||
          p >= q * (b - *x)) {  // golden step
        e = (*x >= xm) ? a - *x : b - *x;
        d = C * e;
      } else {  // parabolic step
        d = p / q;
        const double u = *x + d;
        if (u - a < tol2 || b - u < tol2) d = myriota_brent_sign(tol1, xm - *x);
      }
    } else {
      e = (*x >= xm) ? a - *x : b - *x;
      d = C * e;
    }
    const double u =
        (fabs(d) >= tol1 ? *x + d : *x + myriota_brent_sign(tol1, d));
    const double fu = f(u, fdata);
    if (fu <= *fx) {
      if (u >= *x)
        a = *x;
      else
        b = *x;
      v = w;
      w = *x;
      *x = u;
      fv = fw;
      fw = *fx;
      *fx = fu;
    } else {
      if (u < *x)
        a = u;
      else
        b = u;
      if (fu <= fw || w == *x) {
        v = w;
        w = u;
        fv = fw;
        fw = fu;
      } else if (fu <= fv || v == *x || v == w) {
        v = u;
        fv = fu;
      }
    }
  }
  return max_iterations;  // reached maximum number of iterations.
}

// hidden reverse function used by myriota_rotate
// This implementaiton is based on John Bentley's
// Pearls of Programing refered to at:
// http://stackoverflow.com/questions/22078728/rotate-array-left-or-right-by-a-set-number-of-positions-in-on-complexity
void myriota_rotate_reverse(int *a, int sz) {
  int i, j;
  for (i = 0, j = sz - 1; i < j; i++, j--) {
    int tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
  }
}

void myriota_rotate(int *array, int size, int n) {
  n = myriota_int_mod(n, size);
  myriota_rotate_reverse(array, size - n);
  myriota_rotate_reverse(array + size - n, n);
  myriota_rotate_reverse(array, size);
}

void myriota_msequence(const int N, int *r) {
  // msequence generator polynomials
  static int p3[3] = {0, 1, 1};
  static int p4[4] = {0, 0, 1, 1};
  static int p5[5] = {0, 0, 1, 0, 1};
  static int p6[6] = {0, 0, 0, 0, 1, 1};
  static int p7[7] = {0, 0, 0, 1, 0, 0, 1};
  static int p8[8] = {0, 0, 0, 1, 1, 1, 0, 1};
  static int p9[9] = {0, 0, 0, 0, 1, 0, 0, 0, 1};
  static int p10[10] = {0, 0, 0, 0, 0, 0, 1, 0, 0, 1};
  static int p11[11] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1};
  static int p12[12] = {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1};
  static int p13[13] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1};
  static int p14[14] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1};
  static int *mtaps[15] = {NULL, NULL, NULL, p3,  p4,  p5,  p6, p7,
                           p8,   p9,   p10,  p11, p12, p13, p14};

  if (N < 3 || N > 14)
    error_message_and_exit(
        "Only m-sequences with N > 2 and N < 15 and available.");

  // get taps
  int *taps = mtaps[N];

  int M = (1 << N) - 1;  // M = 2^N-1
  // vector of 1's of length N
  int m[N];
  for (int i = 0; i < N; i++) m[i] = 1;

  for (int ind = 0; ind < M; ind++) {
    int buf = 0;
    for (int i = 0; i < N; i++) buf += taps[i] * m[i];
    buf = buf % 2;            // xor
    myriota_rotate(m, N, 1);  // right shift by 1
    m[0] = buf;
    r[ind] = m[N - 1];
  }
}

myriota_complex myriota_discrete_fourier_transform(const unsigned int N,
                                                   const myriota_complex *in,
                                                   const myriota_decimal f) {
  myriota_complex sum = 0;
  for (int n = 0; n < N; n++) sum += myriota_polar(1, -2 * pi * n * f) * in[n];
  return sum;
}

// FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)
static void myriota_fft_internal(myriota_decimal data[], int nn, int isign) {
  int n, mmax, m, j, istep, i;
  myriota_decimal wtemp, wr, wpr, wpi, wi, theta;
  myriota_decimal tempr, tempi;

  n = nn << 1;
  j = 1;
  for (i = 1; i < n; i += 2) {
    if (j > i) {
      tempr = data[j - 1];
      data[j - 1] = data[i - 1];
      data[i - 1] = tempr;
      tempr = data[j];
      data[j] = data[i];
      data[i] = tempr;
    }
    m = n >> 1;
    while (m >= 2 && j > m) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }
  mmax = 2;
  while (n > mmax) {
    istep = 2 * mmax;
    theta = -2 * pi / (isign * mmax);
    wtemp = sin(0.5 * theta);
    wpr = -2.0 * wtemp * wtemp;
    wpi = sin(theta);
    wr = 1.0;
    wi = 0.0;
    for (m = 1; m < mmax; m += 2) {
      for (i = m; i <= n; i += istep) {
        j = i + mmax;
        tempr = wr * data[j - 1] - wi * data[j];
        tempi = wr * data[j] + wi * data[j - 1];
        data[j - 1] = data[i - 1] - tempr;
        data[j] = data[i] - tempi;
        data[i - 1] += tempr;
        data[i] += tempi;
      }
      wr = (wtemp = wr) * wpr - wi * wpi + wr;
      wi = wi * wpr + wtemp * wpi + wi;
    }
    mmax = istep;
  }
}

// Does forward and inverse ffts,
// isign=1 forward, isign=-1 inverse
static void myriota_fft_internal2(const unsigned int N,
                                  const myriota_complex *in,
                                  myriota_complex *out, int isign) {
  if (!myriota_is_power_of_two(N)) {
    error_message_and_exit(" N must be a power of 2 for FFT \n");
  }
  if (in != out) memcpy(out, in, sizeof(myriota_complex) * N);
  myriota_fft_internal((myriota_decimal *)out, N, isign);
}

void myriota_fft(const unsigned int N, const myriota_complex *in,
                 myriota_complex *out) {
  myriota_fft_internal2(N, in, out, 1);
}

void myriota_inverse_fft(const unsigned int N, const myriota_complex *in,
                         myriota_complex *out) {
  myriota_fft_internal2(N, in, out, -1);
  for (int n = 0; n < N; n++) out[n] /= N;
}

// The complex Phi function.
// Used to compute the periodogram directly from Fourier
// coefficients.
myriota_complex periodogram_phi(const myriota_decimal f, const int N,
                                const int M) {
  // use 2 order approximation if near zero
  if (fabs(f) < 0.01 / N) {
    const myriota_complex a0 = N;
    const myriota_complex a1 = I * pi * f * (N * N - N);
    const myriota_complex a2 =
        pi * pi * f * f * (-2 * N * N * N + 3 * N * N - N) / 3.0;
    return (a0 + a1 + a2) / M;
  }
  const myriota_complex a = myriota_polar(1.0, 2 * pi * f * N) - 1;
  const myriota_complex b = myriota_polar(1.0, 2 * pi * f) - 1;
  return a / b / M;
}

myriota_complex periodogram_time_domain_v(const myriota_decimal f,
                                          const myriota_complex *x,
                                          const int N) {
  myriota_complex s = 0;
  for (int n = 0; n < N; n++) s += x[n] * myriota_polar(1.0, -2 * pi * f * n);
  return s;
}

// Standard periodogram computed from time domain samples
myriota_decimal periodogram_standard(const myriota_decimal f,
                                     const myriota_complex *x, const int N) {
  myriota_complex s = periodogram_time_domain_v(f, x, N);
  return myriota_complex_norm(s) / N;
}

myriota_complex periodogram_frequency_domain_v(const myriota_decimal f,
                                               const myriota_complex *F,
                                               const int N, const int M) {
  myriota_complex v = 0;
  for (int k = 0; k < M; k++)
    v += F[k] * periodogram_phi((1.0 * k) / M - f, N, M);
  return v;
}

// Periodogram computed using frequenct domain samples
myriota_decimal periodogram_frequency_domain(const myriota_decimal f,
                                             const myriota_complex *F,
                                             const int N, const int M) {
  const myriota_complex v = periodogram_frequency_domain_v(f, F, N, M);
  return myriota_complex_norm(v) / N;
}

typedef struct {
  const myriota_complex *F;
  const int N;
  const int M;
} periodogram_brent_data;

double periodogram_brent_frequency_domain(const double f, void *data) {
  periodogram_brent_data *p = data;
  return -periodogram_frequency_domain(f, p->F, p->N, p->M);
}

double periodogram_brent_time_domain(const double f, void *data) {
  periodogram_brent_data *p = data;
  return -periodogram_standard(f, p->F, p->N);
}

void myriota_detect_sinusoid_inplace(myriota_complex *x, const unsigned int N,
                                     myriota_decimal *frequency,
                                     myriota_complex *amplitude,
                                     myriota_decimal *residual_variance,
                                     myriota_decimal *confidence) {
  // compute residual variance
  double sigma2 = 0.0;
  for (int n = 0; n < N; n++) sigma2 += myriota_complex_norm(x[n]) / N;

  // compute fft, mutates input array x
  unsigned int M = myriota_greater_power_of_two(N);
  myriota_fft(M, x, x);

  // find maximiser of the fft
  double Imax = myriota_complex_norm(x[0]);
  unsigned int nhat = 0;
  for (int n = 1; n < M; n++) {
    double thisnorm = myriota_complex_norm(x[n]);
    if (Imax < thisnorm) {
      nhat = n;
      Imax = thisnorm;
    }
  }

  // refine the periodogram using Brents method
  periodogram_brent_data data = {x, N, M};
  double xhat;
  myriota_inverse_fft(M, x, x);
  myriota_brent(periodogram_brent_time_domain, &data, (nhat - 0.5) / M,
                1.0 * nhat / M, (nhat + 0.5) / M, &Imax, &xhat, 1e-6, 100);
  Imax *= -1;  // Brent's method minimises, so need to invert

  // compute confidence (complex version of the Turkman-Walker test)
  // this is a probability that this frequency estimate corresponds with an
  // actual sinusoid, near 1 means it's very likely a sinusoid
  double cN = 2 * log(N) - log(log(N)) + log(3 / pi);
  double MN = Imax / sigma2 - cN / 2;

  *confidence = exp(-exp(-MN));
  *frequency = myriota_fracpart(xhat);
  *residual_variance = fmax(0.0, sigma2 - Imax / N);
  *amplitude = periodogram_time_domain_v(xhat, x, N) / N;
}

void myriota_matrix_multiply(const int M, const int N, const int K,
                             const double *A, const double *B, double *X) {
  for (int m = 0; m < M; m++) {
    for (int k = 0; k < K; k++) {
      double x = 0;
      for (int n = 0; n < N; n++) x += *(A + N * m + n) * *(B + K * n + k);
      *(X + K * m + k) = x;
    }
  }
}

void myriota_matrix_transpose(const int M, const int N, const double *A,
                              double *B) {
  for (int m = 0; m < M; m++)
    for (int n = 0; n < N; n++) *(B + M * n + m) = *(A + N * m + n);
}

#define swap(type, a, b) \
  {                      \
    const type t = a;    \
    a = b;               \
    b = t;               \
  }

#define comp_LU(M, N, A, LU, piv)                                     \
  {                                                                   \
    for (int m = 0; m < M; m++)                                       \
      for (int n = 0; n < N; n++) LU[m][n] = *(A + N * m + n);        \
                                                                      \
    for (int i = 0; i < M; i++) piv[i] = i;                           \
    double pivsign = 1.0;                                             \
                                                                      \
    for (int k = 0; k < N; k++) {                                     \
      int p = k;                                                      \
      for (int i = k + 1; i < M; i++)                                 \
        if (fabs(LU[i][k]) > fabs(LU[p][k])) p = i;                   \
                                                                      \
      if (p != k) {                                                   \
        for (int j = 0; j < N; j++) swap(double, LU[p][j], LU[k][j]); \
        swap(int, piv[p], piv[k]);                                    \
        pivsign = -pivsign;                                           \
      }                                                               \
      if (LU[k][k] != 0.0) {                                          \
        for (int i = k + 1; i < M; i++) {                             \
          LU[i][k] = LU[i][k] / LU[k][k];                             \
          for (int j = k + 1; j < N; j++)                             \
            LU[i][j] = LU[i][j] - LU[i][k] * LU[k][j];                \
        }                                                             \
      }                                                               \
    }                                                                 \
  }

int myriota_matrix_lu(const int M, const int N, const double *A, double *L,
                      double *U, int *piv) {
  if (M < N) return -1;

  double LU[M][N];
  comp_LU(M, N, A, LU, piv);

  // set L
  for (int m = 0; m < M; m++)
    for (int n = 0; n < m; n++) *(L + N * m + n) = LU[m][n];  // lower
  for (int n = 0; n < N; n++) *(L + N * n + n) = 1.0;         // diagonal is 1
  for (int m = 0; m < M; m++)
    for (int n = m + 1; n < N; n++) *(L + N * m + n) = 0.0;  // upper zeros

  // set U
  for (int m = 0; m < N; m++)
    for (int n = 0; n < m; n++) *(U + N * m + n) = 0.0;  // lower zeros
  for (int m = 0; m < N; m++)
    for (int n = m; n < N; n++) *(U + N * m + n) = LU[m][n];  // upper

  return 0;
}

int myriota_matrix_solve(const int N, const int K, const double *A,
                         const double *Y, double *X) {
  double LU[N][N];
  int piv[N];
  comp_LU(N, N, A, LU, piv);

  // check if this matrix is singular
  const double eps = 3e-16;
  for (int n = 0; n < N; n++)
    if (fabs(LU[n][n]) < eps) return -1;

  // Copy NxK Y to pX with pivot
  double pX[N][K];
  for (int n = 0; n < N; n++)
    for (int k = 0; k < K; k++) pX[n][k] = *(Y + K * piv[n] + k);

  // Solve LZ = Y(piv,:)
  for (int k = 0; k < N; k++)
    for (int i = k + 1; i < N; i++)
      for (int j = 0; j < K; j++) pX[i][j] -= pX[k][j] * LU[i][k];

  // Solve UX = Z
  for (int k = N - 1; k >= 0; k--) {
    for (int j = 0; j < K; j++) pX[k][j] /= LU[k][k];
    for (int i = 0; i < k; i++)
      for (int j = 0; j < K; j++) pX[i][j] -= pX[k][j] * LU[i][k];
  }

  // copy result
  for (int n = 0; n < N; n++)
    for (int k = 0; k < K; k++) *(X + K * n + k) = pX[n][k];

  return 0;
}

void myriota_matrix_print(const int M, const int N, const double *A, FILE *f) {
  for (int m = 0; m < M; m++) {
    for (int n = 0; n < N; n++) fprintf(f, "%f ", *(A + N * m + n));
    fprintf(f, "\n");
  }
}

void myriota_polyfit(const double *t, const double *x, const int N, const int r,
                     double *a) {
  double T[N][r + 1];
  for (int n = 0; n < N; n++)
    for (int i = 0; i <= r; i++) T[n][i] = pow(t[n], i);
  double Tt[r + 1][N];
  myriota_matrix_transpose(N, r + 1, &T[0][0], &Tt[0][0]);
  double TtT[r + 1][r + 1];
  myriota_matrix_multiply(r + 1, N, r + 1, &Tt[0][0], &T[0][0], &TtT[0][0]);
  double Tx[r + 1];
  myriota_matrix_multiply(r + 1, N, 1, &Tt[0][0], x, Tx);
  myriota_matrix_solve(r + 1, 1, &TtT[0][0], Tx, a);
}

unsigned int myriota_tlv_size(const void *tlv,
                              unsigned int (*size)(const void *)) {
  if (tlv == NULL) return 0;
  unsigned int s = size(tlv);
  while ((tlv = myriota_tlv_next(tlv, size))) s += size(tlv);
  return s;
}

static bool _true(const void *a, void *b) { return true; }

unsigned int myriota_tlv_count(const void *tlv,
                               unsigned int (*size)(const void *)) {
  return myriota_tlv_count_find(tlv, size, _true, NULL);
}

unsigned int myriota_tlv_count_find(const void *tlv,
                                    unsigned int (*size)(const void *),
                                    bool (*find)(const void *, void *),
                                    void *find_state) {
  if (tlv == NULL) return 0;
  if (size(tlv) == 0) return 0;
  unsigned int c = 0;
  while ((tlv = myriota_tlv_find(tlv, size, find, find_state))) {
    c++;
    tlv = myriota_tlv_next(tlv, size);
  }
  return c;
}

void *myriota_tlv_next(const void *tlv, unsigned int (*size)(const void *)) {
  if (tlv == NULL) return NULL;
  if (size(tlv) == 0) return NULL;
  void *next = (uint8_t *)tlv + size(tlv);
  if (size(next) == 0) return NULL;
  return next;
}

void *myriota_tlv_get(int i, const void *tlv,
                      unsigned int (*size)(const void *)) {
  if (size(tlv) == 0) return NULL;
  if (i <= 0) return (void *)tlv;
  return myriota_tlv_get(i - 1, myriota_tlv_next(tlv, size), size);
}

int myriota_tlv_append(void *tlv, const void *a,
                       unsigned int (*size)(const void *), int (*end)(void *)) {
  if (tlv == NULL) return -1;
  if (a == NULL) return -1;
  if (size(a) == 0) return -1;
  const unsigned int ts = myriota_tlv_size(tlv, size);
  const unsigned int as = size(a);
  memmove((uint8_t *)tlv + ts, a, as);
  end((uint8_t *)tlv + ts + as);
  return 0;
}

int myriota_tlv_delete(void *tlv, void *d, unsigned int (*size)(const void *),
                       int (*end)(void *)) {
  if (size(tlv) == 0) return -1;
  do {  // find d in tlv
    if (d == tlv) break;
  } while ((tlv = myriota_tlv_next(tlv, size)));
  if (tlv == NULL) return -1;  // d iss not in tlv
  void *n = myriota_tlv_next(d, size);
  const unsigned int sn = myriota_tlv_size(n, size);
  memmove(d, n, sn);
  end((uint8_t *)d + sn);
  return 0;
}

void *myriota_tlv_find(const void *tlv, unsigned int (*size)(const void *),
                       bool (*find)(const void *, void *), void *find_state) {
  return myriota_tlv_get_find(0, tlv, size, find, find_state);
}

void *myriota_tlv_get_find(int i, const void *tlv,
                           unsigned int (*size)(const void *),
                           bool (*find)(const void *, void *),
                           void *find_state) {
  if (tlv == NULL) return NULL;
  if (size(tlv) == 0) return NULL;
  int c = -1;
  do {  // find d in tlv
    if (find(tlv, find_state)) c++;
    if (c == i) return (void *)tlv;
  } while ((tlv = myriota_tlv_next(tlv, size)));
  return NULL;
}

int myriota_tlv_filter(const void *tlv, unsigned int (*size)(const void *),
                       bool (*f)(const void *, void *), void *f_state,
                       const void *x[]) {
  if (size(tlv) == 0) return 0;
  int i = 0;
  while (size(tlv) != 0) {
    if (f(tlv, f_state)) {
      x[i] = tlv;
      i++;
    }
    tlv = myriota_tlv_next(tlv, size);
  }
  return i;
}

void *myriota_tlv_from_file(FILE *f, int (*end)(void *)) {
  if (f == NULL) return NULL;
  const int end_size = end(NULL);  // size of terminator
  size_t s = myriota_greater_power_of_two(end_size) * 8;
  size_t r = 0;
  uint8_t *buf = NULL;
  while (!feof(f)) {
    buf = (uint8_t *)realloc(buf, s);
    r += fread(buf + r, 1, s - r - end_size, f);
    s *= 2;  // file continues, get more memory
  }
  end(buf + r);  // add terminator
  return buf;
}

int myriota_sort_unique(void *base, size_t nitems, size_t size,
                        int (*cmp)(const void *, const void *)) {
  if (base == NULL) return 0;
  if (nitems == 0) return 0;
  uint8_t t[size * nitems];
  memcpy(t, base, size * nitems);
  qsort(t, nitems, size, cmp);
  int c = 1;
  memcpy(base, t, size);  // first element is unique
  for (size_t i = 1; i < nitems; i++) {
    if (cmp(t + (i - 1) * size, t + i * size) == 0) continue;
    memcpy(base + c * size, t + i * size, size);
    c++;
  }
  return c;
}
