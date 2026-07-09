/**
 * @file dispatch.h
 * @brief Bytecode dispatcher interface.
 *
 * Provides the execution entry point for interpreting decrypted Ouroboros
 * bytecode programs. The dispatcher supports opcode-based LED and UART actions
 * and halts safely on termination or unknown instructions.
 *
 * @author Kevin Thomas
 * @date 2026-07-09
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef DISPATCH_H
#define DISPATCH_H

#include <stdint.h>

/**
 * @brief Execute a bytecode program from a decrypted buffer.
 *
 * Supported opcodes include END (0x00), LED fill (0x01), TX string (0x03),
 * and MAC marker halt (0xAA). Execution stops on END, MAC marker, invalid
 * boundaries, or unknown opcode.
 *
 * @param program Pointer to bytecode program bytes.
 * @param length Available program length in bytes.
 * @return None
 */
void dispatch_program(const uint8_t *program, uint8_t length);

#endif // DISPATCH_H