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

#include "math/myriotacommon.h"
#include <assert.h>
#include <stdio.h>

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

static int cmp_double(const void *ap, const void *bp) {
  double a = *(double *)ap;
  double b = *(double *)bp;
  if (a > b) return 1;
  if (a < b) return -1;
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
  if (nitems % 2 != 0) return myriota_select_double(nitems / 2, a, nitems);
  const double upper = myriota_select_double(nitems / 2, a, nitems);
  const double lower = myriota_select_double(nitems / 2 - 1, a, nitems);
  return (lower + upper) / 2.0;
}

int32_t myriota_median_int32(int32_t *a, const size_t nitems) {
  if (nitems % 2 != 0) return myriota_select_int32(nitems / 2, a, nitems);
  const int64_t upper = myriota_select_int32(nitems / 2, a, nitems);
  const int64_t lower = myriota_select_int32(nitems / 2 - 1, a, nitems);
  return (lower + upper) / 2;
}

// Public domain SHA-256
// Source: https://github.com/amosnier/sha-2
// For more information, please refer to <http://unlicense.org>

#include <stdint.h>
#include <string.h>

#define CHUNK_SIZE 64
#define TOTAL_LEN_LEN 8

/*
 * ABOUT bool: this file does not use bool in order to be as pre-C99 compatible
 * as possible.
 */

/*
 * Comments from pseudo-code at https://en.wikipedia.org/wiki/SHA-2 are
 * reproduced here. When useful for clarification, portions of the pseudo-code
 * are reproduced here too.
 */

/*
 * Initialize array of round constants:
 * (first 32 bits of the fractional parts of the cube roots of the first 64
 * primes 2..311):
 */
static const uint32_t k[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

struct buffer_state {
  const uint8_t *p;
  size_t len;
  size_t total_len;
  int single_one_delivered; /* bool */
  int total_len_delivered;  /* bool */
};

static inline uint32_t right_rot(uint32_t value, unsigned int count) {
  /*
   * Defined behaviour in standard C for all count where 0 < count < 32,
   * which is what we need here.
   */
  return value >> count | value << (32 - count);
}

static void init_buf_state(struct buffer_state *state, const void *input,
                           size_t len) {
  state->p = input;
  state->len = len;
  state->total_len = len;
  state->single_one_delivered = 0;
  state->total_len_delivered = 0;
}

/* Return value: bool */
static int calc_chunk(uint8_t chunk[CHUNK_SIZE], struct buffer_state *state) {
  size_t space_in_chunk;

  if (state->total_len_delivered) {
    return 0;
  }

  if (state->len >= CHUNK_SIZE) {
    memcpy(chunk, state->p, CHUNK_SIZE);
    state->p += CHUNK_SIZE;
    state->len -= CHUNK_SIZE;
    return 1;
  }

  memcpy(chunk, state->p, state->len);
  chunk += state->len;
  space_in_chunk = CHUNK_SIZE - state->len;
  state->p += state->len;
  state->len = 0;

  /* If we are here, space_in_chunk is one at minimum. */
  if (!state->single_one_delivered) {
    *chunk++ = 0x80;
    space_in_chunk -= 1;
    state->single_one_delivered = 1;
  }

  /*
   * Now:
   * - either there is enough space left for the total length, and we can
   * conclude,
   * - or there is too little space left, and we have to pad the rest of this
   * chunk with zeroes. In the latter case, we will conclude at the next
   * invokation of this function.
   */
  if (space_in_chunk >= TOTAL_LEN_LEN) {
    const size_t left = space_in_chunk - TOTAL_LEN_LEN;
    size_t len = state->total_len;
    int i;
    memset(chunk, 0x00, left);
    chunk += left;

    /* Storing of len * 8 as a big endian 64-bit without overflow. */
    chunk[7] = (uint8_t)(len << 3);
    len >>= 5;
    for (i = 6; i >= 0; i--) {
      chunk[i] = (uint8_t)len;
      len >>= 8;
    }
    state->total_len_delivered = 1;
  } else {
    memset(chunk, 0x00, space_in_chunk);
  }

  return 1;
}

/*
 * Limitations:
 * - Since input is a pointer in RAM, the data to hash should be in RAM, which
 * could be a problem for large data sizes.
 * - SHA algorithms theoretically operate on bit strings. However, this
 * implementation has no support for bit string lengths that are not multiples
 * of eight, and it really operates on arrays of bytes. In particular, the len
 * parameter is a number of bytes.
 */
void myriota_sha256(const void *input, size_t len, uint8_t hash[32]) {
  /*
   * Note 1: All integers (expect indexes) are 32-bit unsigned integers and
   * addition is calculated modulo 2^32. Note 2: For each round, there is one
   * round constant k[i] and one entry in the message schedule array w[i], 0 = i
   * = 63 Note 3: The compression function uses 8 working variables, a through h
   * Note 4: Big-endian convention is used when expressing the constants in this
   * pseudocode, and when parsing message block data from bytes to words, for
   * example, the first word of the input message "abc" after padding is
   * 0x61626380
   */

  /*
   * Initialize hash values:
   * (first 32 bits of the fractional parts of the square roots of the first 8
   * primes 2..19):
   */
  uint32_t h[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                  0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  int i, j;

  /* 512-bit chunks is what we will operate on. */
  uint8_t chunk[64];

  struct buffer_state state;

  init_buf_state(&state, input, len);

  while (calc_chunk(chunk, &state)) {
    uint32_t ah[8];

    /*
     * create a 64-entry message schedule array w[0..63] of 32-bit words
     * (The initial values in w[0..63] don't matter, so many implementations
     * zero them here) copy chunk into first 16 words w[0..15] of the message
     * schedule array
     */
    uint32_t w[64];
    const uint8_t *p = chunk;

    memset(w, 0x00, sizeof w);
    for (i = 0; i < 16; i++) {
      w[i] = (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
             (uint32_t)p[3];
      p += 4;
    }

    /* Extend the first 16 words into the remaining 48 words w[16..63] of the
     * message schedule array: */
    for (i = 16; i < 64; i++) {
      const uint32_t s0 =
          right_rot(w[i - 15], 7) ^ right_rot(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const uint32_t s1 =
          right_rot(w[i - 2], 17) ^ right_rot(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    /* Initialize working variables to current hash value: */
    for (i = 0; i < 8; i++) ah[i] = h[i];

    /* Compression function main loop: */
    for (i = 0; i < 64; i++) {
      const uint32_t s1 =
          right_rot(ah[4], 6) ^ right_rot(ah[4], 11) ^ right_rot(ah[4], 25);
      const uint32_t ch = (ah[4] & ah[5]) ^ (~ah[4] & ah[6]);
      const uint32_t temp1 = ah[7] + s1 + ch + k[i] + w[i];
      const uint32_t s0 =
          right_rot(ah[0], 2) ^ right_rot(ah[0], 13) ^ right_rot(ah[0], 22);
      const uint32_t maj = (ah[0] & ah[1]) ^ (ah[0] & ah[2]) ^ (ah[1] & ah[2]);
      const uint32_t temp2 = s0 + maj;

      ah[7] = ah[6];
      ah[6] = ah[5];
      ah[5] = ah[4];
      ah[4] = ah[3] + temp1;
      ah[3] = ah[2];
      ah[2] = ah[1];
      ah[1] = ah[0];
      ah[0] = temp1 + temp2;
    }

    /* Add the compressed chunk to the current hash value: */
    for (i = 0; i < 8; i++) h[i] += ah[i];
  }

  /* Produce the final hash value (big-endian): */
  for (i = 0, j = 0; i < 8; i++) {
    hash[j++] = (uint8_t)(h[i] >> 24);
    hash[j++] = (uint8_t)(h[i] >> 16);
    hash[j++] = (uint8_t)(h[i] >> 8);
    hash[j++] = (uint8_t)h[i];
  }
}
