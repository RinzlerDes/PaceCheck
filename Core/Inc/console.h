/**
 ******************************************************************************
 * @file    console.h
 * @brief   USART1 operator console on the ST-LINK virtual COM port
 *          (115200 8N1) with printf retarget and a line-read/tokenize
 *          helper (provided driver).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED DRIVER — do not change it, and do not send it as your
 *          own work.
 *
 * ============================================================================
 *  DO-NOT-TOUCH PINS (spec section 7.3)
 *  This driver exclusively owns PB6 (USART1 TX) and PB7 (USART1 RX).
 *  The provided drivers own all of the pins below. Do not reconfigure any of
 *  them at any time, and read-modify-write each shared GPIO register.
 *
 *      PA1  - OLED SPI1 SCK.
 *      PA7  - OLED SPI1 MOSI  *and*  TLC59731 RGB-LED data (shared, JP5 ON).
 *      PH0  - OLED chip select.       PH1  - RGB-LED select.
 *      PC8  - OLED reset.             PC9  - OLED data/command.
 *      PB6  - console USART1 TX.      PB7  - console USART1 RX.
 * ============================================================================
 *
 *  Usage:
 *      console_init();                      // after clock_init()
 *      printf("PC-1000 rev A\r\n");         // printf goes to PuTTY
 *      char line[80]; char *tok[8];
 *      console_read_line(line, sizeof line);        // blocks, echoes,
 *                                                   // handles backspace
 *      int n = console_tokenize(line, tok, 8);      // divides the line on
 *                                                   // whitespace
 *
 *  Notes:
 *   - printf expands '\n' to "\r\n", so PuTTY logs are clean. The log IS
 *     your device-history-record evidence (R5). Keep this behavior.
 *   - %f and %g are NOT available (newlib-nano, float printf disabled). This
 *     omission is by design, because R6 and R7 need integer or fixed-point
 *     statistics. Print tenths as  value/10, value%10  with "%lu.%lu".
 *   - All functions are BLOCKING except console_poll(). Do not call any of
 *     these functions between the first and last captured edge of a run (R10).
 ******************************************************************************
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>

/** Configure PB6/PB7 as USART1 and the UART for 115200 8N1. Call one time. */
void console_init(void);

/** Transmit one character (blocking). The driver expands '\n' to "\r\n". */
void console_putc(char c);

/** Transmit a NUL-terminated string (blocking). */
void console_write(const char *s);

/** Receive one character (blocking). The driver does not echo it. */
char console_getc(void);

/**
 * Non-blocking receive. Returns the next received character (0..255), or -1
 * when no character waits. Use this for "press any key" prompts and for the
 * POST retry loop, because it does not stall your loop.
 */
int console_poll(void);

/**
 * Read one line (blocking) with echo, until the ENTER key. Handles backspace
 * and DEL. Stores maxlen-1 characters at maximum, always NUL-terminates, and
 * echoes "\r\n" at the end. Returns the number of characters that it stored.
 */
int console_read_line(char *buf, size_t maxlen);

/**
 * Divide a line into whitespace-separated tokens IN PLACE. The function
 * changes the line buffer, because each separator becomes a NUL. argv[i]
 * point into the line buffer. Returns the token count (0..max_tokens).
 * A typical use is to parse "e 1500".
 */
int console_tokenize(char *line, char *argv[], int max_tokens);

#endif /* CONSOLE_H */
