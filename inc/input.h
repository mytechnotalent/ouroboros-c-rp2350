/**
 * @file input.h
 * @brief UART input parser and authentication pipeline interface.
 *
 * This header exposes the input buffer and processing entry points used by the
 * firmware to parse terminal input, execute cryptographic verification, and run
 * decrypted bytecode payloads.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

/**
 * @brief Input buffer size in bytes.
 */
#define INPUT_BUFFER_SIZE 32

/**
 * @brief Input buffer used by UART parser and crypto pipeline.
 */
extern uint8_t input_buf[INPUT_BUFFER_SIZE];

/**
 * @brief Poll UART and process one pending input byte.
 *
 * Handles mode transitions, echoing, backspace edits, and submit processing.
 *
 * @param None
 * @return None
 */
void process_uart_input(void);

/**
 * @brief Run authentication pipeline on currently padded input.
 *
 * Derives key material, tests encrypted entries, executes valid bytecode, and
 * presents visual result feedback.
 *
 * @param None
 * @return None
 */
void handle_authentication(void);

/**
 * @brief Execute bytecode stored in the decrypted result buffer.
 *
 * @param None
 * @return None
 */
void execute_bytecode(void);

/**
 * @brief Clear the entire input buffer and reset write index.
 *
 * @param None
 * @return None
 */
void clear_input_buf(void);

#endif // INPUT_H