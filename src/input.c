/**
 * @file input.c
 * @brief UART input parsing and authentication pipeline implementation.
 *
 * Implements character collection, submit/backspace handling, key derivation,
 * table verification, bytecode execution, and result feedback rendering.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "input.h"
#include "config.h"
#include "delay.h"
#include "dispatch.h"
#include "ouroboros.h"
#include "uart.h"
#include "ws2812.h"
#include <stdint.h>
#include <string.h>

#define KEY_PRESS_WHITE_R 0xFF
#define KEY_PRESS_WHITE_G 0xFF
#define KEY_PRESS_WHITE_B 0xFF

/**
 * @brief Encrypted bytecode table copied from the reference implementation.
 */
static const uint8_t table_ciphers[CIPHER_ENTRIES][CIPHER_ENTRY_SIZE] = {
    {0xB3, 0x51, 0x61, 0xFA, 0x78, 0xD4, 0x91, 0xFE, 0xC0, 0x87, 0x82, 0xBD,
     0xDA, 0x87, 0x6E, 0xF9, 0x09, 0x23, 0xE5, 0xA5, 0x69, 0x9F, 0x34, 0x4B,
     0xDD, 0x24, 0xD3, 0x4A, 0x35, 0x96, 0x47, 0x7F, 0xB3, 0x13, 0xFC, 0x6C,
     0x7B, 0x95, 0xAE, 0x25, 0xD1, 0x1C, 0xA5, 0x0A, 0xD7, 0xE1, 0x65, 0x3A},
    {0x5A, 0x9C, 0x67, 0x26, 0xDA, 0xB7, 0x6D, 0x26, 0xB7, 0x48, 0x7B, 0xBC,
     0x26, 0xCA, 0x34, 0x3C, 0x14, 0x72, 0xDE, 0xF4, 0xE5, 0xEE, 0x73, 0xC2,
     0x53, 0xB2, 0xAE, 0x1B, 0xA8, 0xC0, 0xAF, 0xE7, 0xDE, 0x20, 0x31, 0x93,
     0x7E, 0xFB, 0xEF, 0x80, 0xFD, 0x82, 0xF8, 0x7B, 0xF8, 0xDA, 0x59, 0x51}};

/**
 * @brief UART/authentication input buffer.
 */
uint8_t input_buf[INPUT_BUFFER_SIZE];

/**
 * @brief Enter input mode, save previous state, and reset timeout.
 *
 * @param ch Received UART character.
 * @return None
 */
static void enter_input_mode(uint8_t ch) {
  (void)ch;
  if (sys_state != STATE_INPUT)
    prev_state = sys_state;
  sys_state = STATE_INPUT;
  input_timer = 0;
  ws2812_fill(KEY_PRESS_WHITE_R, KEY_PRESS_WHITE_G, KEY_PRESS_WHITE_B);
}

/**
 * @brief Handle one backspace/delete keystroke.
 *
 * @param None
 * @return None
 */
static void handle_backspace(void) {
  if (uart_idx == 0)
    return;
  uart_idx--;
  tx_byte('\b');
  tx_byte(' ');
  tx_byte('\b');
}

/**
 * @brief Append one character if space remains.
 *
 * @param ch Input character.
 * @return None
 */
static void append_char(uint8_t ch) {
  if (uart_idx >= INPUT_BUFFER_SIZE)
    return;
  input_buf[uart_idx++] = ch;
  tx_byte(ch);
}

/**
 * @brief Restore visible state after success/failure feedback.
 *
 * @param success Non-zero on authentication success.
 * @return None
 */
static void restore_after_feedback(uint8_t success) {
  anim_idx = success ? 2 : 1;
  delay_5s();
  sys_state = prev_state;
  anim_timer = 0;
  if (sys_state == STATE_ANIM)
    force_advance();
  else
    render_state();
}

/**
 * @brief Emit success or failure LED feedback and restore state.
 *
 * @param success Non-zero on authentication success.
 * @return None
 */
static void show_result(uint8_t success) {
  if (success)
    ws2812_fill(0xFF, 0xFF, 0x00);
  else
    ws2812_fill(0xFF, 0x00, 0xFF);
  restore_after_feedback(success);
}

/**
 * @brief Try all encrypted entries and keep winning index.
 *
 * @param None
 * @return Non-zero when a valid entry is found.
 */
static uint8_t try_cipher_table(void) {
  uint8_t winner = 0xFF;
  for (uint8_t i = 0; i < CIPHER_ENTRIES; i++) {
    uint8_t pass =
        (uint8_t)ctr_decrypt_verify(table_ciphers[i], NULL, result_buf);
    uint8_t mask = (uint8_t)(0U - pass);
    winner = (uint8_t)((winner & (uint8_t)~mask) | (i & mask));
  }
  if (winner == 0xFF)
    return 0;
  (void)ctr_decrypt_verify(table_ciphers[winner], NULL, result_buf);
  return 1;
}

/**
 * @brief Pad input buffer with zeros to fixed width.
 *
 * @param None
 * @return None
 */
static void pad_input_to_fixed(void) {
  for (uint8_t i = uart_idx; i < INPUT_BUFFER_SIZE; i++)
    input_buf[i] = 0;
}

/**
 * @brief Handle carriage return/line feed submission.
 *
 * @param None
 * @return None
 */
static void handle_submit(void) {
  tx_crlf();
  pad_input_to_fixed();
  handle_authentication();
  tx_prompt();
}

void process_uart_input(void) {
  uint8_t ch;
  if (!uart_has_data())
    return;
  ch = rx_byte();
  if (ch == '\r' || ch == '\n') {
    enter_input_mode(ch);
    handle_submit();
    return;
  }
  if (ch == '\b' || ch == 0x7F) {
    enter_input_mode(ch);
    handle_backspace();
    return;
  }
  if (ch < 0x20 || ch > 0x7E)
    return;
  enter_input_mode(ch);
  append_char(ch);
}

void handle_authentication(void) {
  speck_init_key(input_buf);
  davies_meyer_hash(input_buf, NULL, HASH_ITERATIONS);
  if (try_cipher_table()) {
    execute_bytecode();
    show_result(1);
  } else
    show_result(0);
  clear_input_buf();
  uart_flush_rx();
}

void execute_bytecode(void) { dispatch_program(result_buf, CIPHER_ENTRY_SIZE); }

void clear_input_buf(void) {
  memset(input_buf, 0, sizeof(input_buf));
  uart_idx = 0;
}