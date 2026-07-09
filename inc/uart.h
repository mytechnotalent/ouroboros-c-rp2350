/**
 * @file uart.h
 * @brief UART communication API for terminal I/O and command transport.
 *
 * This header defines UART constants and blocking/non-blocking serial helpers
 * used by the Ouroboros firmware for user input, prompt output, and bytecode
 * string transmission.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * @brief UART baud rate in bits per second.
 */
#define BAUD 9600
/**
 * @brief Initialize UART peripheral for 8N1 serial operation.
 *
 * Configures baud generator and enables TX/RX for terminal interaction.
 *
 * @param None
 * @return None
 */
void uart_console_init(void);

/**
 * @brief Transmit one byte over UART.
 *
 * Blocking transmit helper that waits for UDRE and writes to UDR0.
 *
 * @param data Byte value to transmit.
 * @return None
 */
void tx_byte(uint8_t data);

/**
 * @brief Transmit a null-terminated ASCII string.
 *
 * Sends each character using tx_byte until the string terminator is reached.
 *
 * @param str Pointer to null-terminated source string.
 * @return None
 */
void tx_string(const char *str);

/**
 * @brief Receive one byte from UART.
 *
 * Blocking receive helper that waits until RXC0 indicates data is available.
 *
 * @param None
 * @return Received byte from UDR0.
 */
uint8_t rx_byte(void);

/**
 * @brief Check whether a byte is available in the receive register.
 *
 * @param None
 * @return Non-zero if RXC0 is set, otherwise zero.
 */
uint8_t uart_has_data(void);

/**
 * @brief Emit the command prompt string.
 *
 * Prints the standard terminal prompt used by the firmware.
 *
 * @param None
 * @return None
 */
void tx_prompt(void);

/**
 * @brief Emit carriage return and line feed.
 *
 * @param None
 * @return None
 */
void tx_crlf(void);

/**
 * @brief Flush UART receiver state by toggling RX enable.
 *
 * Clears stale receiver status after long crypto windows or terminal floods.
 *
 * @param None
 * @return None
 */
void uart_flush_rx(void);

#endif // UART_H