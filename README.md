![image](https://github.com/mytechnotalent/ouroboros-c-rp2350/blob/main/Ouroboros-C-RP2350.png?raw=true)

## FREE Reverse Engineering Self-Study Course [HERE](https://github.com/mytechnotalent/Reverse-Engineering-Tutorial)

<br>

# The Ouroboros Engine (C on RP2350)

RP2350 bare-metal C crypto-auth: Ouroboros pipelines across key expansion, Davies-Meyer stretching, CTR decryption, branchless MAC masking, bytecode dispatch.

| Field | Value |
| --- | --- |
| Author | Kevin Thomas |
| Version | 1.0.0 |
| Date | 2026-07-09 |
| Target | RP2350 (`pico2`) |
| Clock | 150 MHz (`SYS_CLOCK_HZ`) |
| Build System | CMake + Pico SDK |
| Language | C11 |
| License | MIT |

## Table of Contents

1. Overview
2. System Architecture
3. Cryptographic Design
4. Hardware Subsystems
5. Bytecode Interpreter
6. Side-Channel-Relevant Implementation Notes
7. State Machine
8. Boot Sequence
9. Build and Flash
10. File Structure
11. Notes

## Overview

The RP2350 Ouroboros port receives a passphrase over serial stdio, derives key material using Speck-128/256 and Davies-Meyer style iteration, decrypts flash-resident encrypted bytecode entries in CTR mode, validates integrity via branchless MAC masking, and executes the recovered payload through a minimal interpreter.

High-level flow:

1. Collect user input into a fixed 32-byte buffer.
2. Expand Speck round keys from the input key.
3. Run 24,576 iterative hashing rounds over a 16-byte hash buffer.
4. Decrypt and verify every encrypted entry (constant iteration count).
5. Re-decrypt winner, dispatch bytecode, and render visual feedback.

## System Architecture

```text
                    +-------------------------------------------+
                    |              RP2350 @ 150 MHz             |
                    |                                           |
USB serial stdio -> | process_uart_input()                      |
                    |      |                                    |
                    |      v                                    |
                    | input_buf[32]                             |
                    |      |                                    |
                    |      v                                    |
                    | speck_init_key() -> round_keys[272]       |
                    |      |                                    |
                    |      v                                    |
                    | davies_meyer_hash() -> hash_buf[16]       |
                    |      |                                    |
                    |      v                                    |
                    | try_cipher_table() over CIPHER_ENTRIES    |
                    | (decrypt + branchless MAC mask each)      |
                    |      |                                    |
                    |      v                                    |
                    | dispatch_program(result_buf, 48)          |
                    |      |                          |         |
                    |      v                          v         |
                    |   tx_byte()/tx_string()     ws2812_fill() |
                    |                              (LED_COUNT)  |
                    +-------------------------------------------+
```

## Cryptographic Design

### Speck-128/256

The implementation uses 128-bit blocks, 256-bit keys, and 34 rounds.

Round function per step:

$$
x' = ((x \ggg 8) + y) \oplus k_i
$$

$$
y' = (y \lll 3) \oplus x'
$$

The key schedule produces 34 round keys (272 bytes total in memory).

### Davies-Meyer Iterative Stretching

The hash buffer starts from a fixed 16-byte IV and is iteratively encrypted:

$$
H_0 = IV,\quad H_j = E_K(H_{j-1}),\quad j \in [1, 24576]
$$

After iteration, the feed-forward xor is applied:

$$
\hat{H} = H_{24576} \oplus IV
$$

The resulting 16-byte value is retained in `hash_buf` and repeated to form 32-byte output material when requested.

### CTR-Mode Decryption

Each candidate entry is 48 bytes (3 blocks of 16). For block counter b in {0,1,2}, the counter block is:

$$
CTR_b = \hat{H}[0:8] \parallel b \parallel 0^7
$$

Keystream is Speck-encrypted and xored with ciphertext block-by-block.

### Branchless MAC Mask Verification

Bytes 16..47 are expected to equal 0xAA. Differences are or-reduced into one byte and converted to a full-byte mask:

$$
mask =
\begin{cases}
0xFF, & diff = 0 \\
0x00, & diff \neq 0
\end{cases}
$$

Then every output byte is masked. Failed MAC means the full 48-byte output becomes zero.

### Constant-Iteration Cipher Scan

All entries are always processed. Winner selection is delayed and branchless by mask composition, avoiding early-exit behavior tied to match location.

## Hardware Subsystems

### WS2812 LED Driver

- Transport: RP2350 PIO program (`ws2812.pio`).
- Data pin: GPIO26.
- Chain length: 64 LEDs (`LED_COUNT`).
- Pixel packing: GRB on wire.
- Post-frame reset delay: 80 us.

### Button Logic

- Input pin: GPIO15 (active-low with pull-up).
- Debounce: software delay + release wait.
- State cycle: ANIM -> RED -> GREEN -> BLUE -> ANIM.
- During INPUT mode, button state changes are ignored.

### UART/Console Interface

- Uses Pico stdio (`stdio_init_all`) for console IO.
- Input parser supports printable ASCII, backspace/delete, and CR/LF submit.
- Prompt string: `> `

### Timer and Delay Behavior

- Main loop delay primitive uses busy-wait microseconds.
- Animation threshold: 1349 ticks.
- Input timeout threshold: 44280 ticks.
- Success/failure feedback hold: 5 seconds.

## Bytecode Interpreter

Dispatcher executes linear bytecode from `result_buf` and halts on invalid/terminal opcodes.

| Opcode | Mnemonic | Payload | Behavior |
| --- | --- | --- | --- |
| 0x00 | END | none | Stop execution |
| 0x01 | LED_FILL | R, G, B | Fill all LEDs |
| 0x03 | TX_STR | len, bytes... | Transmit string bytes |
| 0xAA | MAC marker | none | Stop execution |

The VM is intentionally minimal: no jumps, no stack, no dynamic allocation, bounded forward parse only.

## Side-Channel-Relevant Implementation Notes

The implementation includes several hardening-aligned behaviors:

- Variable micro-jitter before cryptographic steps (0..7 loop counts from timer seed).
- Constant-time byte comparison utility.
- Constant-iteration entry scan.
- Branchless MAC mask generation and full-buffer masking.
- Input buffer zeroization after each authentication attempt.
- RX flush after authentication completion.

These notes describe implementation behavior only; they are not formal attack-resistance claims.

## State Machine

```text
          +-------+    button    +-----+    button    +-------+    button    +------+
          | ANIM  | -----------> | RED | -----------> | GREEN | -----------> | BLUE |
          +---+---+              +--+--+              +---+---+              +--+---+
              ^                     |                     |                     |
              +---------------------+---------------------+---------------------+
                                     wrap to ANIM

Any mode + serial keypress -> INPUT
INPUT + ENTER -> authenticate -> YELLOW (success) or PURPLE (fail) for 5s -> restore previous mode
INPUT + inactivity timeout -> restore previous mode
```

| State | Value | LED behavior |
| --- | --- | --- |
| ANIM | 0 | Rotating R/G/B frames |
| RED | 1 | Solid red |
| GREEN | 2 | Solid green |
| BLUE | 3 | Solid blue |
| INPUT | 4 | White while typing |

## Boot Sequence

Initialization order in `main`:

1. Disable watchdog (`clear_reset_flags`).
2. Configure clock and GPIO pins (`config_pins`).
3. Initialize serial console (`uart_console_init`).
4. Reset runtime state variables and counters.
5. Clear cryptographic buffers (`crypto_init`).
6. Initialize WS2812 PIO path (`ws2812_init`).
7. Render initial state LEDs.
8. Emit prompt.
9. Enter perpetual main loop (`delay_and_timers`, `handle_button`).

## Build and Flash

### Prerequisites

1. Raspberry Pi Pico SDK available to CMake (installed via VS Code Pico extension or exported `PICO_SDK_PATH`).
2. CMake and a supported ARM embedded toolchain.
3. Pico tooling (for UF2 copy or direct load path).

### Build

```bash
cmake -S . -B build -DPICO_BOARD=pico2
cmake --build build
```

Expected primary output:

- `build/ouroboros-c-rp2350.uf2`

### Run/Load

This workspace includes VS Code tasks for compile and load workflows:

- Compile Project
- Run Project
- Flash
- Rescue Reset
- RISC-V Reset (RP2350)

## File Structure

```text
ouroboros-c-rp2350/
|-- CMakeLists.txt
|-- inc/
|   |-- button.h
|   |-- config.h
|   |-- delay.h
|   |-- dispatch.h
|   |-- input.h
|   |-- ouroboros.h
|   |-- uart.h
|   |-- ws2812.h
|-- src/
|   |-- button.c
|   |-- config.c
|   |-- delay.c
|   |-- dispatch.c
|   |-- input.c
|   |-- main.c
|   |-- ouroboros.c
|   |-- uart.c
|   |-- ws2812.c
|   |-- ws2812.pio
|-- build/
```

<br>

See [LICENSE](https://github.com/mytechnotalent/ouroboros-c-rp2350/blob/main/LICENSE).
