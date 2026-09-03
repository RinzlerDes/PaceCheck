/**
 ******************************************************************************
 * @file    oled.h
 * @brief   Text API for the on-board 0.96" 128x64 OLED (SSD1315 on SPI1)
 *          (provided driver).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED DRIVER — do not change it, and do not send it as your
 *          own work.
 *
 * ============================================================================
 *  DO-NOT-TOUCH PINS (spec section 7.3)
 *  This driver exclusively owns PA1 (SPI1 SCK), PA7 (SPI1 MOSI — shared with
 *  the RGB LED data line, JP5 ON), PH0 (chip select), PC9 (data/command) and
 *  PC8 (reset). The provided drivers own all of the pins below. Do not
 *  reconfigure any of them at any time, and read-modify-write each shared
 *  GPIO register.
 *
 *      PA1  - OLED SPI1 SCK.
 *      PA7  - OLED SPI1 MOSI  *and*  TLC59731 RGB-LED data (shared, JP5 ON).
 *      PH0  - OLED chip select.       PH1  - RGB-LED select.
 *      PC8  - OLED reset.             PC9  - OLED data/command.
 *      PB6  - console USART1 TX.      PB7  - console USART1 RX.
 *
 *  A wholesale write to GPIOA->MODER kills this display mid-demo (R3/§7.3).
 * ============================================================================
 *
 *  Text grid: 8 rows (0 = top) x 21 columns, 5x7 font in a 6x8 cell.
 *
 *  Cost model (R9/R10): one oled_write_line() call transfers ~136 bytes at
 *  8 MHz SPI, which takes about 200 us. That is cheap in an operator loop
 *  and RUINOUS in a capture path with edges 100 us apart (D8). Call the OLED
 *  from your main loop only. Do not call it from an ISR, and do not call it
 *  mid-run in CAL-VERIFY.
 *
 *  Usage:
 *      oled_init();                          // after clock_init()
 *      oled_clear();
 *      oled_write_line(0, "PC-1000  ACQUIRING");
 *      oled_printf(2, "N %4lu  last %lu us", n, dt);
 ******************************************************************************
 */
#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#define OLED_ROWS 8u   /* text rows, 0 = top      */
#define OLED_COLS 21u  /* characters in each row  */

/**
 * Reset and initialize the panel. Call it one time, after clock_init().
 * The call takes about 120 ms.
 */
void oled_init(void);

/** Clear the full display (all 8 rows). */
void oled_clear(void);

/**
 * Write one full text row (0..7). The driver truncates or space-pads the
 * string to 21 columns, so a shorter string cleanly replaces the previous
 * line. The driver shows an unknown character as a space.
 */
void oled_write_line(uint8_t row, const char *text);

/** printf-style convenience wrapper around oled_write_line(). */
void oled_printf(uint8_t row, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#endif /* OLED_H */
