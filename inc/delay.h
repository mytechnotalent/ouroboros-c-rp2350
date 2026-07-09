/**
 * @file delay.h
 * @brief Timing primitives and software scheduler helpers.
 *
 * This header defines delay thresholds and exported timer orchestration
 * functions used by the main loop to drive animation cadence and input timeout
 * behavior.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/**
 * @brief Animation update threshold in software ticks.
 */
#define ANIM_DELAY_THRESHOLD 1349U

/**
 * @brief Input inactivity timeout threshold in software ticks.
 */
#define INPUT_TIMEOUT 44280U

/**
 * @brief Execute a calibrated busy-wait loop.
 *
 * @param count Iteration count used to scale delay duration.
 * @return None
 */
void delay(uint16_t count);

/**
 * @brief Run periodic delay and update all software timers.
 *
 * @param None
 * @return None
 */
void delay_and_timers(void);

/**
 * @brief Advance animation index and render current animation frame.
 *
 * @param None
 * @return None
 */
void force_advance(void);

/**
 * @brief Block execution for an approximately five-second interval.
 *
 * @param None
 * @return None
 */
void delay_5s(void);

#endif // DELAY_H