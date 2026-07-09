/**
 * @file main.c
 * @brief Firmware entry point and system bootstrap sequence.
 *
 * Initializes memory, hardware peripherals, shared runtime state, and enters
 * the perpetual main loop for timer and input handling.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "button.h"
#include "config.h"
#include "delay.h"
#include "ouroboros.h"
#include "uart.h"
#include "ws2812.h"

/**
 * @brief Initialize hardware and runtime state.
 *
 * @param None
 * @return None
 */
static void init_system(void) {
  clear_reset_flags();
  config_pins();
  uart_console_init();
  sys_state = STATE_ANIM;
  uart_idx = 0;
  anim_idx = 0;
  prev_state = 0;
  anim_timer = 0;
  input_timer = 0;
  crypto_init();
  ws2812_init();
  render_state();
  tx_prompt();
}

/**
 * @brief Execute the endless main loop.
 *
 * @param None
 * @return None
 */
static void main_loop(void) {
  while (1) {
    delay_and_timers();
    handle_button();
  }
}

/**
 * @brief Program entry point.
 *
 * @param None
 * @return Always returns 0; function effectively does not return in runtime.
 */
int main(void) {
  init_system();
  main_loop();
  return 0;
}