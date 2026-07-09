/**
 * @file ws2812.c
 * @brief WS2812 signaling and state rendering implementation.
 *
 * Implements PIO-driven waveform generation and high-level system-state color
 * rendering for the LED chain.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#include "ws2812.h"
#include "config.h"
#include "ws2812.pio.h"
#include <hardware/clocks.h>
#include <hardware/pio.h>
#include <pico/time.h>
#include <stdint.h>

#define WS2812_RESET_US 80U
#define WS2812_STARTUP_RESET_US 1000U
#define WS2812_FREQ_HZ 800000U

static PIO ws2812_pio = pio0;
static uint ws2812_sm = 0;

/**
 * @brief Pack RGB bytes into wire order used by the original implementation.
 *
 * @param r Red component in range 0..255.
 * @param g Green component in range 0..255.
 * @param b Blue component in range 0..255.
 * @return Packed pixel value in bits 23..0.
 */
static uint32_t pack_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)g << 16U) | ((uint32_t)r << 8U) | (uint32_t)b;
}

/**
 * @brief Push one WS2812 pixel into the PIO TX FIFO.
 *
 * @param pixel Packed 24-bit pixel value.
 * @return None
 */
static void put_pixel(uint32_t pixel) {
  pio_sm_put_blocking(ws2812_pio, ws2812_sm, pixel << 8U);
}

/**
 * @brief Configure one PIO state machine for WS2812 timing.
 *
 * @param pio PIO instance.
 * @param sm State machine index.
 * @param offset Loaded program offset.
 * @param pin WS2812 data pin.
 * @return None
 */
static void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin) {
  pio_gpio_init(pio, pin);
  pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
  pio_sm_config c = ws2812_program_get_default_config(offset);
  sm_config_set_sideset_pins(&c, pin);
  sm_config_set_out_shift(&c, false, true, 24);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
  uint cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div = (float)clock_get_hz(clk_sys) / (WS2812_FREQ_HZ * cycles_per_bit);
  sm_config_set_clkdiv(&c, div);
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);
}

void ws2812_init(void) {
  uint offset = pio_add_program(ws2812_pio, &ws2812_program);
  ws2812_program_init(ws2812_pio, ws2812_sm, offset, LED_PIN);
  busy_wait_us_32(WS2812_STARTUP_RESET_US);
  ws2812_fill(0x00, 0x00, 0x00);
  busy_wait_us_32(WS2812_STARTUP_RESET_US);
}

void ws2812_fill(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t pixel = pack_rgb(r, g, b);
  for (uint8_t i = 0; i < LED_COUNT; i++)
    put_pixel(pixel);
  busy_wait_us_32(WS2812_RESET_US);
}

void animate_colors(uint8_t color_index) {
  if (color_index == 0)
    ws2812_fill(0xFF, 0x00, 0x00);
  else if (color_index == 1)
    ws2812_fill(0x00, 0xFF, 0x00);
  else
    ws2812_fill(0x00, 0x00, 0xFF);
}

void render_state(void) {
  if (sys_state == STATE_INPUT)
    return;
  if (sys_state == STATE_ANIM)
    animate_colors(anim_idx);
  else if (sys_state == STATE_RED)
    ws2812_fill(0xFF, 0x00, 0x00);
  else if (sys_state == STATE_GREEN)
    ws2812_fill(0x00, 0xFF, 0x00);
  else if (sys_state == STATE_BLUE)
    ws2812_fill(0x00, 0x00, 0xFF);
}