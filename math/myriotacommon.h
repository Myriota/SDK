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

#ifndef MYRIOTA_MATH_COMMON_H
#define MYRIOTA_MATH_COMMON_H

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

// double, float, etc. Default is double.
// define MYRIOTA_DECIMAL at compile time if you want to change this.
#ifndef MYRIOTA_DECIMAL
#define MYRIOTA_DECIMAL double
#endif

// Force a compilation error if condition is true
#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))
#endif

#define field_size(type, member) sizeof(((type *)0)->member)

// Get number of element in an array
#define NUM_ELEMS(array) (sizeof(array) / sizeof((array)[0]))

// Myriota's floating point and complex floating point types
typedef MYRIOTA_DECIMAL myriota_decimal;
typedef MYRIOTA_DECIMAL _Complex myriota_complex;

// Approximation of the irrational number
#define pi 3.14159265358979323846

// square root of 2.
#define sqrt2 1.41421356237

// square root of 2 divided by 2.
// equivalently the reciprocal of sqrt(2.0)
#define sqrt2on2 (sqrt2 / 2.0)

#define myriota_free(x) (free((void *)x))

#ifdef __cplusplus
extern "C" {
#endif

// Print error function, line, message and then exit
#ifndef error_message_and_exit
#define error_message_and_exit(message)                            \
  do {                                                             \
    fprintf(stderr, "!!%s %d: %s\n", __func__, __LINE__, message); \
    fflush(stderr);                                                \
    while (1) {                                                    \
      exit(EXIT_FAILURE);                                          \
    }                                                              \
  } while (0)
#endif

// Returns true if a string contains only hexidecimal characters
bool myriota_is_hex(const char *s);

// Converts two hexidecimal characters to byte, i.e number in the
// interval [0, 255].
// Returns 2 (the number of characters scanned) on success and 0 if
// either character is not hexidecimal
int myriota_hex_to_byte(const char *h, uint8_t *b);

// Parses a hexidecimal string into a buffer of bytes.
// Returns the number of hexidecimal digits parsed and zero
// if scan failed, i.e. if the string is not valid hex.
// The string must contain an even number of character or
// 0 is returned and no parsing takes place
int myriota_hex_to_buf(const char *s, void *buf);

// Like myriota_hex_to_buf but scans at most n characters
// The smaller of n and strlen(s) must be even or 0 is returned
int myriota_n_hex_to_buf(const char *s, const size_t n, void *buf);

// Writes buffer in hexidecimal format to string. s should been
// allocated with size at least 2*buf_size+1. Returns number of hexidecimal
// characters written
int myriota_buf_to_hex(const void *buf, const size_t buf_size, char *s);

// Print buffer to standard out in hex format
void myriota_print_hex(const void *buf, int size_bytes);

// Standard 32-bit cyclic redudunacy check.
uint32_t myriota_crc32(const void *data, size_t length, uint32_t offset);

// Return the kth largest element from the array of size nitems
double myriota_select_double(const int k, double *a, const size_t nitems);
int32_t myriota_select_int32(const int k, int32_t *a, const size_t nitems);

// Returns the median of an array of size nitems
double myriota_median_double(double *a, const size_t nitems);
int32_t myriota_median_int32(int32_t *a, const size_t nitems);

#define SHA256_BYTES 32
// 256 bit secure hash algorithm
// Output written to hash of size SHA256_BYTES
void myriota_sha256(const void *input, size_t len, uint8_t hash[SHA256_BYTES]);

#ifdef __cplusplus
}
#endif

#endif
