/**
 * @file dispatch.c
 * @brief Bytecode dispatcher execution engine.
 *
 * Decodes and executes decrypted bytecode instructions for LED rendering and
 * UART string output with safe bounds checks.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "dispatch.h"
#include "uart.h"
#include "ws2812.h"
#include <stdint.h>

/**
 * @brief Decode and execute LED opcode payload.
 *
 * @param p Program byte pointer.
 * @param l Total program length.
 * @param i Current instruction index pointer.
 * @return 1 when payload execution succeeds, otherwise 0.
 */
static uint8_t run_led(const uint8_t *p, uint8_t l, uint8_t *i) {
  if ((uint8_t)(*i + 3U) > l)
    return 0;
  ws2812_fill(p[*i], p[*i + 1U], p[*i + 2U]);
  *i = (uint8_t)(*i + 3U);
  return 1;
}

/**
 * @brief Decode and execute TX_STR opcode payload.
 *
 * @param p Program byte pointer.
 * @param l Total program length.
 * @param i Current instruction index pointer.
 * @return 1 when payload execution succeeds, otherwise 0.
 */
static uint8_t run_tx(const uint8_t *p, uint8_t l, uint8_t *i) {
  uint8_t n;
  if (*i >= l)
    return 0;
  n = p[(*i)++];
  if ((uint8_t)(*i + n) > l)
    return 0;
  for (uint8_t c = 0; c < n; c++)
    tx_byte(p[(*i)++]);
  return 1;
}

/**
 * @brief Execute one opcode and update instruction index.
 *
 * @param p Program byte pointer.
 * @param l Total program length.
 * @param i Current instruction index pointer.
 * @return 1 to continue dispatch loop, otherwise 0.
 */
static uint8_t step_dispatch(const uint8_t *p, uint8_t l, uint8_t *i) {
  uint8_t op = p[(*i)++];
  if (op == 0x00 || op == 0xAA)
    return 0;
  if (op == 0x01)
    return run_led(p, l, i);
  if (op == 0x03)
    return run_tx(p, l, i);
  return 0;
}

void dispatch_program(const uint8_t *program, uint8_t length) {
  uint8_t idx = 0;
  while (idx < length && step_dispatch(program, length, &idx)) {
  }
}