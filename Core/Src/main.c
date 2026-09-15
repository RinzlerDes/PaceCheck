/**
 ******************************************************************************
 * @file    main.c
 * @brief   Starter smoke test — REPLACE THIS FILE with your PaceCheck
 *          code.
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *
 *  What this program does, and all that it does:
 *   1. Sets the system clock to 64 MHz            (clock_init)
 *   2. Starts the UART console at 115200 8N1      (console_init)
 *   3. Starts the OLED and prints a banner        (oled_init)
 *   4. Starts the RGB LED                          (rgb_init)
 *   5. Starts the provided LOOPBACK stimulus       (loopback_start):
 *      a 1 kHz square wave on Arduino D2 = PD12. Jumper D2 to D5, and
 *      your future capture engine has an at-home signal source.
 *   6. Loops: it reads a console line, tokenizes it, echoes the tokens
 *      back, steps the LED color, and mirrors the first token on the OLED.
 *      Type 'menu' to open the provided CONFIG console shell (menu.c).
 *
 *  Flash it with no changes first. If you see the banner in PuTTY, text on
 *  the OLED, and the LED that steps through colors at each ENTER key, then
 *  your board, toolchain and terminal all work, before you write one line
 *  of firmware. This unchanged build IS the pre-S5 environment smoke test
 *  for each person (spec section 10, item 0). Build it, flash it, open
 *  PuTTY, and get a TA check-off before the C exam on Sep 9.
 *
 *  Reminder (spec section 7.3): the provided drivers own PA1, PA7, PH0,
 *  PH1, PC8, PC9, PB6, PB7. Your code must not reconfigure those pins at
 *  any time, and it must read-modify-write each shared GPIO register. Your
 *  pin for this project: PB10 (SENSE, TIM2_CH3 AF1).
 ******************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "capture_engine.h"
#include "clock.h"
#include "console.h"
#include "loopback.h"
#include "menu.h"
#include "oled.h"
#include "rgb_led.h"
#include "stm32wb5mxx.h"
#include "stm32wbxx.h"

int main(void) {
  clock_init();
  console_init();
  oled_init();
  rgb_init();
  capture_engine_init();

  printf("\n");
  printf("PC-1000 PaceCheck starter -- SWEN 563 / CMPE 663\n");
  printf("SYSCLK = %lu Hz (TIM2 kernel clock; 1 us tick needs PSC = ?)\n",
         (unsigned long)clock_sysclk_hz());

  (void)loopback_start(1000u);
  printf(
      "LOOPBACK running: 1000 us square on PD12 (Arduino D2) -- "
      "jumper D2 to D5 for the capture-path self-test.\n");
  printf(
      "Type a line (e.g. 'e 1500') and press Enter; "
      "'menu' enters the CONFIG shell.\n\n");

  oled_write_line(0, "PC-1000  PaceCheck");
  oled_write_line(2, "Starter smoke test");
  oled_write_line(4, "Console: 115200 8N1");
  rgb_set(0u, 0u, 32u);

  static const uint8_t colors[][3] = {
      {32u, 0u, 0u},
      {0u, 32u, 0u},
      {0u, 0u, 32u},
      {32u, 32u, 32u},
  };
  unsigned color_idx = 0u;

  // R1
  char line[80];
  char* tok[8];
  oled_clear();
  uint32_t startup_reset_counter = 0;

  for (;;) {
    my_flag = false;
    uint32_t startup = TIM2->CNT;
    uint32_t startup_period = 100000;

    while ((TIM2->CNT - startup) < startup_period) {
      if (my_flag) {
        break;
      }
    }

    if (my_flag) {
        oled_clear();
      break;
    }

    char* message = "NO DUT SIGNAL";
    char* message2 = "(no edge within 100 ms)";
    char* message3 = "> keypress to retry";
    char message4[21];
    snprintf(message4, sizeof(message4), "retry count: %u", startup_reset_counter);

    printf("%s\n", message);
    printf("%s\n", message2);
    printf("%s\n", message4);
    printf("%s\n", message3);

    oled_write_line(0, message);
    oled_write_line(1, message2);
    oled_write_line(3, message4);
    oled_write_line(4, message3);

    console_getc();
    startup_reset_counter++;
  }
  capture_engine_init();

  for (;;) {
    char line[80];
    char* tok[8];

    printf("> ");
    console_read_line(line, sizeof line);

    int argc = console_tokenize(line, tok, 8);
    if (argc == 1 && strcmp(tok[0], "menu") == 0) {
      menu_loop(); /* does not return (R11)        */
    }
    printf("%d token(s)\n", argc);
    for (int i = 0; i < argc; i++) {
      printf("  [%d] \"%s\"\n", i, tok[i]);
    }

    oled_printf(6, "You typed: %s", (argc > 0) ? tok[0] : "(nothing)");

    rgb_set(colors[color_idx][0], colors[color_idx][1], colors[color_idx][2]);
    color_idx = (color_idx + 1u) % 4u;
  }
}
