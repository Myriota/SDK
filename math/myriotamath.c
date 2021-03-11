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
  if (b == 0) return abs(a);
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
    if (abs(r[i].q) > qmax) return r[i - 1];
    if (fabs(x * r[i].q - r[i].p) < fabs(r[i].q * tol)) return r[i];
  }
  return r[k - 1];
}

// optimised bisection that prevents double computation of f
static double _bisection(double (*f)(double, void *), void *fdata,
                         const double a, const double b, const int sa,
                         const int sb, const double tol) {
  const double m = (a + b) / 2;
  if (fabs(b - a) < tol) return m;
  const int sm = myriota_signum(f(m, fdata));
  if (sm == sa) return _bisection(f, fdata, m, b, sm, sb, tol);
  return _bisection(f, fdata, a, m, sa, sm, tol);
}

double myriota_bisection(double (*f)(double, void *), void *fdata,
                         const double a, const double b, const double tol) {
  const int sa = myriota_signum(f(a, fdata));
  const int sb = myriota_signum(f(b, fdata));
  return _bisection(f, fdata, a, b, sa, sb, tol);
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

double myriota_solve(double (*f)(double, void *), void *fdata, const double y,
                     const double ax, const double bx, const double tol) {
  struct solve_struct s = {f, fdata, y};
  return myriota_bisection(solve_offset, &s, ax, bx, tol);
}

double myriota_minimise(double (*f)(double, void *), void *fdata,
                        const double a, const double b, const double tol) {
  if (fabs(b - a) < tol) return (a + b) / 2;
  const double at = (2 * a + b) / 3;
  const double bt = (a + 2 * b) / 3;
  if (f(at, fdata) > f(bt, fdata))
    return myriota_minimise(f, fdata, at, b, tol);
  return myriota_minimise(f, fdata, a, bt, tol);
}

double myriota_unwrap(const double value, const double previous_value) {
  const double d = myriota_fracpart_scaled(value - previous_value, 2 * pi);
  if (d > pi) return d + previous_value - 2 * pi;
  if (d < -pi) return d + previous_value + 2 * pi;
  return d + previous_value;
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
                                                   const double f) {
  myriota_complex sum = 0;
  for (int n = 0; n < N; n++) sum += myriota_polar(1, -2 * pi * n * f) * in[n];
  return sum;
}

// FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)
static void myriota_fft_internal(double data[], int nn, int isign) {
  int n, mmax, m, j, istep, i;
  double wtemp, wr, wpr, wpi, wi, theta;
  double tempr, tempi;

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
  myriota_fft_internal((double *)out, N, isign);
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
