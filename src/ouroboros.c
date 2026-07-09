/**
 * @file ouroboros.c
 * @brief Cryptographic core implementation for key derivation and verification.
 *
 * Implements key schedule setup, reversible block transforms, iterative hash
 * stretching, constant-time compare, and branchless MAC-masked entry checks.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "ouroboros.h"
#include "config.h"
#include <stdint.h>
#include <string.h>
#if defined(__AVR__)
#include <avr/io.h>
#else
#include <pico/time.h>
#endif

/**
 * @brief Expanded round key storage.
 */
uint8_t round_keys[272]; // 34 rounds * 8 bytes each
/**
 * @brief Hash/nonce workspace buffer.
 */
uint8_t hash_buf[16]; // 128-bit hash output
/**
 * @brief Decrypted output workspace buffer.
 */
uint8_t result_buf[48];

/**
 * @brief Initial vector bytes copied from assembly reference data.
 */
static const uint8_t iv_const[16] = {0x6A, 0x09, 0xE6, 0x67, 0xBB, 0x67,
                                     0xAE, 0x85, 0x3C, 0x6E, 0xF3, 0x72,
                                     0xA5, 0x4F, 0xF5, 0x3A};

/**
 * @brief Read low-entropy timer byte for jitter delay.
 *
 * @param None
 * @return Timer-derived byte.
 */
static uint8_t jitter_seed(void) {
#if defined(__AVR__)
  return TCNT0;
#else
  return (uint8_t)time_us_32();
#endif
}

/**
 * @brief Inject short variable delay in the range 0..7 loop steps.
 *
 * @param None
 * @return None
 */
static void hardware_jitter_delay(void) {
  int8_t n = (int8_t)(jitter_seed() & 0x07U);
  while (n-- >= 0) {
  }
}

/**
 * @brief Load one little-endian 64-bit word from byte array.
 *
 * @param p Pointer to 8-byte source.
 * @return Packed 64-bit value.
 */
static uint64_t load64_le(const uint8_t *p) {
  uint64_t v = 0;
  for (uint8_t i = 0; i < 8; i++)
    v |= ((uint64_t)p[i]) << (8U * i);
  return v;
}

/**
 * @brief Store one 64-bit value to little-endian byte array.
 *
 * @param p Pointer to 8-byte destination.
 * @param v Source 64-bit value.
 * @return None
 */
static void store64_le(uint8_t *p, uint64_t v) {
  for (uint8_t i = 0; i < 8; i++)
    p[i] = (uint8_t)(v >> (8U * i));
}

/**
 * @brief Rotate 64-bit value right by one byte.
 *
 * @param v Input value.
 * @return Rotated value.
 */
static uint64_t ror8_64(uint64_t v) { return (v >> 8U) | (v << 56U); }

/**
 * @brief Rotate 64-bit value left by one byte.
 *
 * @param v Input value.
 * @return Rotated value.
 */
static uint64_t rol8_64(uint64_t v) { return (v << 8U) | (v >> 56U); }

/**
 * @brief Rotate 64-bit value left by three bits.
 *
 * @param v Input value.
 * @return Rotated value.
 */
static uint64_t rol3_64(uint64_t v) { return (v << 3U) | (v >> 61U); }

/**
 * @brief Rotate 64-bit value right by three bits.
 *
 * @param v Input value.
 * @return Rotated value.
 */
static uint64_t ror3_64(uint64_t v) { return (v >> 3U) | (v << 61U); }

/**
 * @brief Build CTR counter block from hash nonce prefix.
 *
 * @param counter Block counter byte.
 * @param ctr Pointer to 16-byte output buffer.
 * @return None
 */
static void make_ctr_block(uint8_t counter, uint8_t *ctr) {
  memset(ctr, 0, BLOCK_SIZE);
  memcpy(ctr, hash_buf, 8);
  ctr[8] = counter;
}

/**
 * @brief Create equality mask from accumulated difference byte.
 *
 * @param diff OR-accumulated difference.
 * @return 0xFF when diff is zero, otherwise 0x00.
 */
static uint8_t equal_mask(uint8_t diff) {
  return (uint8_t)((((uint16_t)diff - 1U) >> 8U) & 0xFFU);
}

/**
 * @brief Verify MAC bytes and mask output branchlessly.
 *
 * @param output Decrypted data buffer.
 * @return 1 if MAC is valid, otherwise 0.
 */
static int mask_by_mac(uint8_t *output) {
  uint8_t diff = 0;
  for (uint8_t i = 16; i < CIPHER_ENTRY_SIZE; i++)
    diff |= (uint8_t)(output[i] ^ 0xAAU);
  uint8_t mask = equal_mask(diff);
  for (uint8_t i = 0; i < CIPHER_ENTRY_SIZE; i++)
    output[i] &= mask;
  return (mask == 0xFFU);
}

void speck_init_key(const uint8_t *key) {
  uint64_t k = load64_le(key);
  uint64_t l_buf[3] = {load64_le(key + 8), load64_le(key + 16),
                       load64_le(key + 24)};
  store64_le(round_keys, k);
  for (uint8_t round = 0; round < 33; round++) {
    uint8_t idx = (uint8_t)(round % 3U);
    uint64_t l = ror8_64(l_buf[idx]);
    l += k;
    l ^= round;
    l_buf[idx] = l;
    k = rol3_64(k);
    k ^= l;
    store64_le(round_keys + ((uint16_t)(round + 1U) * 8U), k);
  }
}

void speck_encrypt(const uint8_t *input, uint8_t *output) {
  uint64_t x = load64_le(input);
  uint64_t y = load64_le(input + 8);
  for (uint8_t round = 0; round < 34; round++) {
    hardware_jitter_delay();
    x = ror8_64(x);
    x += y;
    x ^= load64_le(round_keys + ((uint16_t)round * 8U));
    y = rol3_64(y);
    y ^= x;
  }
  store64_le(output, x);
  store64_le(output + 8, y);
}

void speck_decrypt(const uint8_t *input, uint8_t *output) {
  uint64_t x = load64_le(input);
  uint64_t y = load64_le(input + 8);
  for (uint8_t round = 34; round > 0; round--) {
    uint64_t rk = load64_le(round_keys + ((uint16_t)(round - 1U) * 8U));
    y ^= x;
    y = ror3_64(y);
    x ^= rk;
    x -= y;
    x = rol8_64(x);
  }
  store64_le(output, x);
  store64_le(output + 8, y);
}

void davies_meyer_hash(const uint8_t *input, uint8_t *output,
                       uint32_t iterations) {
  uint8_t block[BLOCK_SIZE];
  (void)input;
  memcpy(hash_buf, iv_const, sizeof(hash_buf));
  for (uint32_t i = 0; i < iterations; i++) {
    hardware_jitter_delay();
    speck_encrypt(hash_buf, block);
    memcpy(hash_buf, block, sizeof(hash_buf));
  }
  for (uint8_t i = 0; i < 16; i++)
    hash_buf[i] ^= iv_const[i];
  if (output != NULL)
    for (uint8_t i = 0; i < KEY_SIZE; i++)
      output[i] = hash_buf[i & 0x0FU];
}

int constant_time_compare(const uint8_t *a, const uint8_t *b, size_t len) {
  uint8_t diff = 0;
  for (size_t i = 0; i < len; i++)
    diff |= (uint8_t)(a[i] ^ b[i]);
  return diff;
}

int ctr_decrypt_verify(const uint8_t *encrypted_data, const uint8_t *key,
                       uint8_t *output) {
  uint8_t ctr[BLOCK_SIZE];
  uint8_t stream[BLOCK_SIZE];
  (void)key;
  for (uint8_t block = 0; block < 3; block++) {
    make_ctr_block(block, ctr);
    speck_encrypt(ctr, stream);
    for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
      uint8_t idx = (uint8_t)(block * BLOCK_SIZE + i);
      output[idx] = (uint8_t)(encrypted_data[idx] ^ stream[i]);
    }
  }
  return mask_by_mac(output);
}

void crypto_init(void) {
  memset(round_keys, 0, sizeof(round_keys));
  memset(hash_buf, 0, sizeof(hash_buf));
  memset(result_buf, 0, sizeof(result_buf));
}