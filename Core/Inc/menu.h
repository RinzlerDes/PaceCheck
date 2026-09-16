/**
 ******************************************************************************
 * @file    menu.h
 * @brief   CONFIG console shell — the prompt, the settings display,
 *          and the parse-and-dispatch loop that uses the provided console
 *          line-reader.
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED SHELL — unlike the black-box drivers (spec section 6),
 *          menu.c is YOURS to complete and to send as your work. The fenced
 *          STUDENT sections in menu.c are the graded content. The shell
 *          plumbing is not graded.
 *
 *  What the shell does for you (plumbing):
 *   - the "PC-1000> " prompt, the line read, the tokenize step, and
 *     the strict numeric parsing.
 *   - the settings storage with the R4 defaults (E 1000, T 50).
 *   - the active-window echo before each run (R4).
 *   - the command dispatch: show / e / t / run / loop / help.
 *   - the endless re-run loop (R11). A reset is not necessary at any time.
 *
 *  What stays yours (fenced STUDENT sections in menu.c):
 *   - the R4 validators: the range rules, T < E, and each explanatory
 *     message that rejects an entry.
 *   - the run hook: your capture engine (R2), the histogram report (R5),
 *     the statistics (R6/R7), the R8 verdict LED, and the R9/R10 display
 *     discipline.
 ******************************************************************************
 */
#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <stdint.h>

#define TOLERANCE_MAX 500
#define TOLERANCE_MIN 10
#define PERIOD_MAX 2000
#define PERIOD_MIN 100

/* R4: the interval count is fixed. A qualification is always full-count. */
#define MENU_N_FIXED 1000u

typedef struct {
    uint32_t E;   /* expected interval, us            */
    uint32_t T;   /* tolerance, us (window [E-T,E+T]) */
    uint32_t N;   /* interval count (MENU_N_FIXED)    */
} menu_settings_t;

/**
 * Run the operator console forever (R11). It reads lines, dispatches
 * commands, and calls the validator code and the run code that you write
 * in the fenced sections. It does not return.
 */
void menu_loop(void);

/** The current settings, for your code. */
const menu_settings_t *menu_settings(void);

#endif /* MENU_H */
