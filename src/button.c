/**
 * @file button.c
 * @brief Button sampling, debounce, and mode-cycling implementation.
 *
 * Implements active-low button polling logic, release synchronization, and
 * state transitions that mirror the UI behavior expected by the firmware.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "button.h"
#include "config.h"
#include "delay.h"
#include "input.h"
#include "ws2812.h"
#include <hardware/gpio.h>
#include <stdint.h>

/**
 * @brief Wait until the active-low button is released.
 *
 * @param None
 * @return None
 */
static void wait_button_release(void) {
  while (button_pressed()) {
  }
}

/**
 * @brief Advance state machine one step and redraw LEDs.
 *
 * @param None
 * @return None
 */
static void cycle_state(void) {
  if (sys_state == STATE_INPUT)
    return;
  sys_state++;
  if (sys_state >= STATE_INPUT) {
    sys_state = STATE_ANIM;
    anim_idx = 0;
  }
  render_state();
}

uint8_t button_pressed(void) { return (uint8_t)!gpio_get(BUTTON_PIN); }

button_state_t poll_button_state(void) {
  return button_pressed() ? BUTTON_SINGLE : BUTTON_NONE;
}

void handle_button(void) {
  if (!button_pressed()) {
    process_uart_input();
    return;
  }
  delay(DEBOUNCE_DELAY);
  if (!button_pressed()) {
    process_uart_input();
    return;
  }
  wait_button_release();
  cycle_state();
}