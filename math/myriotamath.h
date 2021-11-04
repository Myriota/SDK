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

#ifndef MYRIOTA_MATH_H
#define MYRIOTA_MATH_H

#include <assert.h>
#include <complex.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "math/myriotacommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// The sign of x, zero if x is zero.
static inline double myriota_signum(double x) { return (x > 0) - (x < 0); }

// Square of x, i.e., x*x
static inline double myriota_sqr(double x) { return x * x; }

// Cube of x, i.e., x*x*x
static inline double myriota_cub(double x) { return x * x * x; }

// Returns smallest power of 2 greater than or equal to this integer
unsigned int myriota_greater_power_of_two(unsigned int x);
uint64_t myriota_greater_power_of_two_long(uint64_t x);

// Returns 1 if x is a power of 2, zero otherwise
static inline int myriota_is_power_of_two(unsigned int x) {
  return myriota_greater_power_of_two(x) == x;
}
static inline int myriota_is_power_of_two_long(uint64_t x) {
  return myriota_greater_power_of_two_long(x) == x;
}

// Get the nth bit from buffer x
unsigned int myriota_get_bit(const unsigned int n, const uint8_t *x);

//  Set the value of the nth bit of buffer x
void myriota_set_bit(const unsigned int n, const unsigned int value,
                     uint8_t *x);

// write from[0:end-start+1] into to[start,end]
void myriota_write_bits(const uint8_t *from, uint8_t *to,
                        const unsigned int start, const unsigned int stop);

// read from[start:end] into to[0,end-start+1]
void myriota_read_bits(const uint8_t *from, uint8_t *to,
                       const unsigned int start, const unsigned int stop);

// Returns a complex number from rectangular coordinates, i.e.
// from real and imaginary parts.
myriota_complex myriota_rectangular(double re, double im);

// Complex number from magnitude and phase in radians
myriota_complex myriota_polar(double magnitude, double phase);

// Norm (magnitude squared) of a complex number
double myriota_complex_norm(myriota_complex x);

// Magnitude (absolute value) of a complex number
double myriota_complex_abs(myriota_complex x);

// Phase/argument/angle of a complex number
double myriota_complex_arg(myriota_complex x);

// Conjugate of a complex number
myriota_complex myriota_complex_conj(myriota_complex x);

// real and imaginary parts
double myriota_complex_real(myriota_complex x);
double myriota_complex_imag(myriota_complex x);

// Sinc function
double myriota_sinc(double t);

// Blackman window function of width 2W
double myriota_blackman(double t, double W);

// Hyperbolic sine function
double myriota_sinh(double x);

// Inverse cosine using inverse sine
static inline double myriota_acos(double x) { return pi / 2 - asin(x); }

// Factorial of an integer n*(n-1)*(n-2)*...*1
unsigned long myriota_factorial(unsigned int n);

// Returns the value of the nth modified Bessel function at x.
double myriota_besselI(int n, double x);

// Parses a base64 string into a buffer of bytes.
// The length of s (i.e. strlen(s)) must be a multiple of 4. Returns the number
// of base64 characters parsed and zero if scan failed, i.e. if the string is
// not valid base64.
int myriota_base64_to_buf(const char *s, void *buf);

// Like myriota_base64_to_buf but scans precisely n characters.
// Returns the number of bytes written to buf
int myriota_n_base64_to_buf(const char *s, const size_t n, void *buf);

// Writes buffer in base64 format to string. s should be allocated with size at
// least buf_size*4/3+1. buf_size must be a multiple of 3. Returns number of
// hexidecimal characters written.
int myriota_buf_to_base64(const void *buf, const size_t buf_size, char *s);

// Parses a zbase32 string into a buffer of bytes.
// buf should be of size at least strlen(s)*5/8+1. Returns the number
// of zbase32 characters parsed and zero if scan failed, i.e. if the string is
// not valid zbase32.
int myriota_zbase32_to_buf(const char *s, void *buf);

// Like myriota_zbase32_to_buf but scans precisely n characters.
// Returns the number of bytes written to buf
int myriota_n_zbase32_to_buf(const char *s, const size_t n, void *buf);

// Writes buffer in zbase32 format to string. buf_size must be a multiple of 5
// or -1 is returned. s should be allocated with size at least buf_size*8/5+1.
// Returns number of zbase32 characters written.
int myriota_buf_to_zbase32(const void *buf, const size_t buf_size, char *s);

static inline double degrees_to_radians(const double x) { return x * pi / 180; }

static inline double radians_to_degrees(const double x) { return x * 180 / pi; }

// minimum of two integers
static inline int myriota_int_min(int a, int b) { return a < b ? a : b; }

// maximum of two integers
static inline int myriota_int_max(int a, int b) { return a > b ? a : b; }

// Number uniformly distributed on the interval [0,1]
static inline double myriota_random_uniform() {
  return ((double)rand()) / RAND_MAX;
}

// Generate a Bernoulli observation with success probability p
// That is, returns 1 with probability p and 0 with probabiliy 1-p
int myriota_random_bernoulli(double p);

// Generate a geometric observation with success probability p
// That is, the number Bernoulli trials with probability p until a signal
// success (i.e. 1).
int myriota_random_geometric(double p);

// Generate a normally distributed random variable.
// Uses the Box Muller method
double myriota_random_normal();

// Generate an exponentially distributed random variable
double myriota_random_exponential(const double mean);

// Sample from discrete probability mass function p of length n given uniformly
// sampled u from [0,1]. p will be normalised to sum to one prior to sampling
int myriota_random_discrete(const double *p, int n, double u);

// Computes continued fraction expansion of decimal number of size N.
// Returns upper bound on approximation error.
double myriota_continued_fraction(double x, unsigned int size, int *r);

// Greatest common divisor between two integers
long long gcd(long long a, long long b);

typedef struct {
  long long p;  // numerator
  long long q;  // denominator
} myriota_rational;

// construct rational number equivalent to a/b. Numerator and denominator
// of resulting rational will always be relatively prime.
myriota_rational make_myriota_rational(long long a, long long b);

// The sum of two rational numbers
myriota_rational myriota_rational_sum(myriota_rational a, myriota_rational b);

// Return 1 if a > b, -1 if a < b, and 0 if a == b
int myriota_rational_compare(myriota_rational a, myriota_rational b);

// Return first size best rational approximations to decimal x. Output written
// into b
void myriota_best_approximations(double x, unsigned int size,
                                 myriota_rational *b);

// Finds a rational approximation p/q to x satisfying either |x - p/q| < tol or
// |q| < qmax or is the kth best approximation (convergent) of x
myriota_rational myriota_rational_approximation(double x, double tol, int qmax,
                                                unsigned int k);

// Complex 16 bit fixed point type
typedef struct {
  int16_t re;
  int16_t im;
} myriota_complex_16;

// Complex 32 bit fixed point type
typedef struct {
  int32_t re;
  int32_t im;
} myriota_complex_32;

// Clip signed 32 bit integer into interval [-2^15, 2^15)
static inline int16_t myriota_clip_16(const int32_t x) {
  if (x > (1 << 15) - 1) return (1 << 15) - 1;
  if (x < -(1 << 15)) return -(1 << 15);
  return x;
}

// Complex addition of myriota_complex_32
static inline myriota_complex_32 myriota_complex_sum_32(
    const myriota_complex_32 a, const myriota_complex_32 b) {
  return (myriota_complex_32){a.re + b.re, a.im + b.im};
}

#ifdef __cplusplus
}

#include <complex>
#include <stdexcept>
#include <vector>

namespace myriota {

typedef std::complex<double> complex;

// Type generic circular buffer class that supports sequential write and
// random access.
//
// Internally, the size of the buffer is always increased to the power
// of 2 greater than or equal to the requested size. This allows faster
// mod size operations (bitwise AND).
template <typename T>
class CircularBuffer {
 public:
  unsigned int size;
  unsigned int mask;

  // Creates a circular buffer and initialises all
  // entries to init parameter.
  CircularBuffer(unsigned int size, T init)
      : size(myriota_greater_power_of_two(size + 1)),
        mask(this->size - 1),
        buf(this->size, init),
        N(0){};

  // Copy constructor (only works with default initialisation)
  CircularBuffer(const CircularBuffer &b)
      : size(myriota_greater_power_of_two(b.size + 1)),
        mask(b.size - 1),
        buf(b.size, T()),
        N(0){};

  // Write/push an element to the end of the buffer
  inline void push(const T &elem) {
    buf[N & mask] = elem;
    N++;
  }

  // The total number of elements that have been pushed into this buffer.
  inline uint64_t pushed() const { return N; }

  // The maximum and minimum values of n for which this(n) is valid
  inline int64_t maxn() const { return N - 1; }
  inline int64_t minn() const { return N - size; }

  // Read the nth element of the buffer
  inline const T &operator()(const int64_t n) const { return buf[n & mask]; }

  void set(const int64_t n, const T &v) {
    if (n >= minn() && n <= maxn())
      buf[n & mask] = v;
    else
      throw std::out_of_range("circluar buffer set " + std::to_string(n) +
                              " outside [" + std::to_string(minn()) + ", " +
                              std::to_string(maxn()) + "]");
  }

 protected:
  std::vector<T> buf;
  uint64_t N;
};  // namespace myriota

template <typename T>
class Resample {
 public:
  const double W;  // window width
  const myriota_rational r;
  const double gamma;
  const double kappa;
  const double delta;
  const int xi;
  const int gmin;
  const int gmax;
  // Widow width W can be adjusted, larger is slower, but more accurate
  Resample(double in_rate, double out_rate, double W, T zero)
      : W(W),
        r(myriota_rational_approximation(out_rate / in_rate, 1e-6, 1000, 10)),
        gamma((1.0 * r.p) / r.q),
        kappa(fmin(1, gamma)),
        delta(fmax(1, gamma)),
        xi(r.p > r.q ? r.p : r.q),
        gmin(ceil(-xi * W)),
        gmax(floor(xi * W)),
        a(ceil((2 * W) / kappa + 1), zero){};
  void push(T x) { a.push(x); };
  int64_t pushed() const { return a.pushed(); }
  virtual T operator()(int64_t n) const = 0;
  int64_t minn() const { return ceil(gamma * (a.maxn() - a.size) + delta * W); }
  int64_t maxn() const { return floor(gamma * (a.maxn() - 1) - delta * W); }

 protected:
  CircularBuffer<T> a;
};

class ResampleDouble : public Resample<complex> {
 public:
  // Widow width W can be adjusted, larger is slower, but more accurate
  ResampleDouble(double in_rate, double out_rate, double W = 30);
  virtual complex operator()(int64_t n) const override;

 protected:
  std::vector<double> g_buf;
  inline double g(int64_t n) const { return g_buf[n - gmin]; };
};

// Resample 16-bit fixed point input sequence
class Resample16 : public Resample<myriota_complex_16> {
 public:
  const int32_t alpha;
  // Widow width W can be adjusted, larger is slower, but more accurate
  Resample16(double in_rate, double out_rate, double W = 30);
  myriota_complex_16 operator()(int64_t n) const override;
  // nth output sample at 32 bit precision
  myriota_complex_32 n32(int64_t n) const;
  double beta() const;        // fixed point scaling paramter
  double g(int64_t n) const;  // double precision filter
  // pack and rescale output to 16 bit range
  const std::vector<int32_t> &taps() { return f_buf; };  // get filter taps

 protected:
  std::vector<int32_t> f_buf;
  inline int32_t f(int64_t n) const { return f_buf[n - gmin]; };
};

// Resample 16-bit first point input sequence, "avoids" integer division at
// the expense of possible increased clipping for 16-bit inputs taking values
// near the maximum +-2^15.
class Resample16shift : public Resample16 {
 public:
  const int s;  // shift
  Resample16shift(double in_rate, double out_rate, double W = 30)
      : Resample16(in_rate, out_rate, W), s(floor(log2(alpha))){};
  myriota_complex_16 operator()(int64_t n) const override;
};

// Returns int x modulo int y, i.e., the coset representative from
// {0,1,...,y-1} of x from the group Z/y.
//
// Different from x % y when x is negative. This is not the same
// as the remainder of x after division by y when x is negative
//
// The templated type T must implement the % and + operators, e.g. int64_t.
template <class T>
T mod(const T &x, const T &y) {
  const T t = x % y;
  if (t < 0) return t + y;
  return t;
}
}  // namespace myriota
#endif

#endif
