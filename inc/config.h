/**
 * @file config.h
 * @brief Hardware configuration, pin assignments, and shared runtime state.
 *
 * This header centralizes platform-level definitions for the RP2350 target
 * used by the Ouroboros C implementation running at 150 MHz. It defines the
 * runtime system state machine, GPIO pin mappings, shared global state
 * variables, and initialization routines required during firmware startup.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/**
 * @brief Runtime system modes for the Ouroboros user interface.
 *
 * The finite-state machine coordinates idle animation, manual color override
 * modes, and UART input/authentication mode.
 */
typedef enum {
  STATE_ANIM = 0,
  STATE_RED = 1,
  STATE_GREEN = 2,
  STATE_BLUE = 3,
  STATE_INPUT = 4
} system_state_t;

/**
 * @brief Button input pin on RP2350 GPIO.
 */
#define BUTTON_PIN 15U
/**
 * @brief WS2812 data output pin on RP2350 GPIO.
 */
#define LED_PIN 26U
/**
 * @brief UART RX pin alias.
 */
#define UART_RX 1U
/**
 * @brief UART TX pin alias.
 */
#define UART_TX 0U

/**
 * @brief Target RP2350 system clock in hertz.
 */
#define SYS_CLOCK_HZ 150000000U

/**
 * @brief Current runtime system state.
 */
extern uint8_t sys_state;
/**
 * @brief UART input buffer write index.
 */
extern uint8_t uart_idx;
/**
 * @brief Animation color sub-index in the range 0..2.
 */
extern uint8_t anim_idx;
/**
 * @brief Saved state captured before switching into input mode.
 */
extern uint8_t prev_state;
/**
 * @brief Animation software timer counter.
 */
extern uint16_t anim_timer;
/**
 * @brief Input inactivity timer counter.
 */
extern uint16_t input_timer;

/**
 * @brief Configure firmware GPIO directions and default output levels.
 *
 * Initializes pin modes required by the application, including the WS2812 data
 * output and active-low button input with pull-up.
 *
 * @param None
 * @return None
 */
void config_pins(void);

/**
 * @brief Clear reset flags and disable the watchdog timer.
 *
 * Performs the required sequence to clear MCUSR and reliably disable the WDT
 * after watchdog-triggered resets, preventing repeated reset loops.
 *
 * @param None
 * @return None
 */
void clear_reset_flags(void);

#endif // CONFIG_H