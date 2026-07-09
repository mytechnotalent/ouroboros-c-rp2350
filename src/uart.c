/**
 * @file uart.c
 * @brief UART peripheral implementation for blocking serial communication.
 *
 * Provides initialization and transmit/receive helpers used by terminal input
 * handling and dispatcher output paths.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "uart.h"
#include "config.h"
#include <pico/stdlib.h>
#include <stdint.h>

static int buffered_rx = PICO_ERROR_TIMEOUT;

void uart_console_init(void) { stdio_init_all(); }

void tx_byte(uint8_t data) { (void)putchar_raw((int)data); }

void tx_string(const char *str) {
  while (*str)
    tx_byte((uint8_t)*str++);
}

uint8_t rx_byte(void) {
  int ch;
  if (buffered_rx != PICO_ERROR_TIMEOUT) {
    ch = buffered_rx;
    buffered_rx = PICO_ERROR_TIMEOUT;
    return (uint8_t)ch;
  }
  while ((ch = getchar_timeout_us(1000)) == PICO_ERROR_TIMEOUT)
    tight_loop_contents();
  return (uint8_t)ch;
}

uint8_t uart_has_data(void) {
  if (buffered_rx != PICO_ERROR_TIMEOUT)
    return 1U;
  buffered_rx = getchar_timeout_us(0);
  return (uint8_t)(buffered_rx != PICO_ERROR_TIMEOUT);
}

void tx_prompt(void) { tx_string("> "); }

void tx_crlf(void) {
  tx_byte('\r');
  tx_byte('\n');
}

void uart_flush_rx(void) {
  buffered_rx = PICO_ERROR_TIMEOUT;
  while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {
  }
}