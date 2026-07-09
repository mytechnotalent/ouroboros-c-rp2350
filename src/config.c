/**
 * @file config.c
 * @brief Hardware configuration and reset/watchdog bootstrap routines.
 *
 * Implements global runtime state storage, GPIO pin initialization, and
 * watchdog-safe reset flag handling for startup.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "config.h"
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/watchdog.h>

uint8_t sys_state;
uint8_t uart_idx;
uint8_t anim_idx;
uint8_t prev_state;
uint16_t anim_timer;
uint16_t input_timer;

void config_pins(void) {
  set_sys_clock_khz(SYS_CLOCK_HZ / 1000U, true);
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 0);
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);
}

void clear_reset_flags(void) { watchdog_disable(); }