/**
 * @file ouroboros.h
 * @brief Core cryptographic API for Ouroboros authentication.
 *
 * This header defines block/key dimensions, encrypted table sizes, global
 * cryptographic buffers, and the public functions used by the firmware to
 * derive keys, decrypt candidate entries, and validate MAC content.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef OUROBOROS_H
#define OUROBOROS_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Cipher block size in bytes.
 */
#define BLOCK_SIZE 16

/**
 * @brief Cipher key size in bytes.
 */
#define KEY_SIZE 32

/**
 * @brief Key stretching iteration count.
 */
#define HASH_ITERATIONS 24576

/**
 * @brief Number of encrypted payload entries.
 */
#define CIPHER_ENTRIES 2

/**
 * @brief Number of bytes per encrypted table entry.
 */
#define CIPHER_ENTRY_SIZE 48

/**
 * @brief Expanded round key storage.
 */
extern uint8_t round_keys[272]; // 34 rounds * 8 bytes each

/**
 * @brief Hash/nonce workspace buffer.
 */
extern uint8_t hash_buf[16]; // 128-bit hash output

/**
 * @brief Decrypted output buffer.
 */
extern uint8_t result_buf[48];

/**
 * @brief Expand key material into round key schedule.
 *
 * @param key Pointer to KEY_SIZE bytes of input key material.
 * @return None
 */
void speck_init_key(const uint8_t *key);

/**
 * @brief Encrypt one block using the prepared round keys.
 *
 * @param input Pointer to BLOCK_SIZE plaintext bytes.
 * @param output Pointer to BLOCK_SIZE output ciphertext bytes.
 * @return None
 */
void speck_encrypt(const uint8_t *input, uint8_t *output);

/**
 * @brief Decrypt one block using the prepared round keys.
 *
 * @param input Pointer to BLOCK_SIZE ciphertext bytes.
 * @param output Pointer to BLOCK_SIZE plaintext bytes.
 * @return None
 */
void speck_decrypt(const uint8_t *input, uint8_t *output);

/**
 * @brief Perform iterative hash-based key stretching.
 *
 * @param input Pointer to input bytes used as seed material.
 * @param output Pointer to output key buffer.
 * @param iterations Number of rounds to execute.
 * @return None
 */
void davies_meyer_hash(const uint8_t *input, uint8_t *output,
                       uint32_t iterations);

/**
 * @brief Compare two buffers in constant time.
 *
 * @param a Pointer to first buffer.
 * @param b Pointer to second buffer.
 * @param len Number of bytes to compare.
 * @return 0 when equal, otherwise non-zero.
 */
int constant_time_compare(const uint8_t *a, const uint8_t *b, size_t len);

/**
 * @brief Decrypt one candidate entry and apply branchless MAC masking.
 *
 * @param encrypted_data Pointer to CIPHER_ENTRY_SIZE encrypted bytes.
 * @param key Pointer to KEY_SIZE key bytes.
 * @param output Pointer to output buffer receiving decrypted bytes.
 * @return 1 when MAC content is valid, otherwise 0.
 */
int ctr_decrypt_verify(const uint8_t *encrypted_data, const uint8_t *key,
                       uint8_t *output);

/**
 * @brief Initialize cryptographic buffers to a clean state.
 *
 * @param None
 * @return None
 */
void crypto_init(void);

#endif // OUROBOROS_H