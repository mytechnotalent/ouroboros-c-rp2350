/**
 * @file button.h
 * @brief Button handling and mode-cycling interface.
 *
 * Defines the button state enumeration and the public routines used to read
 * the active-low button, poll for state, and integrate button behavior into
 * the main firmware control loop.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

/**
 * @brief Button state classification for compatibility with prior interfaces.
 */
typedef enum {
  BUTTON_NONE = 0,
  BUTTON_SINGLE = 1,
  BUTTON_DOUBLE = 2,
  BUTTON_TRIPLE = 3
} button_state_t;

/**
 * @brief Debounce loop count constant.
 */
#define DEBOUNCE_DELAY 40000U

/**
 * @brief Read the current active-low button state.
 *
 * @param None
 * @return Non-zero when the button is pressed, otherwise zero.
 */
uint8_t button_pressed(void);

/**
 * @brief Handle button debounce, mode transitions, and UART fallback polling.
 *
 * @param None
 * @return None
 */
void handle_button(void);

/**
 * @brief Poll button and convert current reading to compatibility enum.
 *
 * @param None
 * @return BUTTON_SINGLE when pressed, otherwise BUTTON_NONE.
 */
button_state_t poll_button_state(void);

#endif // BUTTON_H