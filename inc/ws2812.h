/**
 * @file ws2812.h
 * @brief WS2812 LED transport and state-rendering interface.
 *
 * Provides high-level rendering routines used to display firmware state on a
 * WS2812 LED chain.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef WS2812_H
#define WS2812_H

#include <stdint.h>

/**
 * @brief Number of WS2812 LEDs in the chain.
 */
#define LED_COUNT 64

/**
 * @brief Initialize WS2812 output pin state.
 *
 * @param None
 * @return None
 */
void ws2812_init(void);

/**
 * @brief Fill all LEDs with one RGB value.
 *
 * @param r Red component in range 0..255.
 * @param g Green component in range 0..255.
 * @param b Blue component in range 0..255.
 * @return None
 */
void ws2812_fill(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Draw an animation color frame by index.
 *
 * @param color_index Animation index in range 0..2.
 * @return None
 */
void animate_colors(uint8_t color_index);

/**
 * @brief Render LEDs from the current runtime system state.
 *
 * @param None
 * @return None
 */
void render_state(void);

#endif // WS2812_H