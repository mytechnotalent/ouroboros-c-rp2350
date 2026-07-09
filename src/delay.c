/**
 * @file delay.c
 * @brief Delay loops and software timer scheduler implementation.
 *
 * Provides calibrated delay primitives and updates animation/input timeout
 * counters used by the main firmware loop.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "delay.h"
#include "config.h"
#include "input.h"
#include "uart.h"
#include "ws2812.h"
#include <pico/time.h>
#include <stdint.h>

/**
 * @brief Tick animation timer and advance when threshold is reached.
 *
 * @param None
 * @return None
 */
static void tick_animation(void) {
  if (++anim_timer < ANIM_DELAY_THRESHOLD)
    return;
  anim_timer = 0;
  if (sys_state == STATE_ANIM)
    force_advance();
}

/**
 * @brief Restore state after input timeout and redraw LEDs.
 *
 * @param None
 * @return None
 */
static void timeout_restore(void) {
  tx_crlf();
  tx_prompt();
  clear_input_buf();
  sys_state = prev_state;
  input_timer = 0;
  anim_timer = 0;
  if (sys_state == STATE_ANIM)
    force_advance();
  else
    render_state();
}

/**
 * @brief Tick input timeout only while in input mode.
 *
 * @param None
 * @return None
 */
static void tick_input_timeout(void) {
  if (sys_state != STATE_INPUT)
    return;
  if (++input_timer < INPUT_TIMEOUT)
    return;
  timeout_restore();
}

void delay(uint16_t count) { busy_wait_us_32((uint32_t)((count + 1U) / 2U)); }

void force_advance(void) {
  anim_timer = 0;
  sys_state = STATE_ANIM;
  anim_idx = (uint8_t)((anim_idx + 1) % 3);
  render_state();
}

void delay_5s(void) { sleep_ms(5000); }

void delay_and_timers(void) {
  delay(1349);
  tick_animation();
  tick_input_timeout();
}