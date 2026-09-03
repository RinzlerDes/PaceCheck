/**
 ******************************************************************************
 * @file    menu.c
 * @brief   CONFIG console shell (provided plumbing + fenced STUDENT stubs).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED SHELL — complete the fenced STUDENT sections, then send
 *          this file as your work. menu.h tells you which parts the course
 *          provides and which parts are yours.
 *
 *  The shell compiles and runs as shipped. All STUDENT stubs return a safe
 *  default and print on the console what the code does not do. No part of
 *  this file hangs. No part of it completes a demo row for you.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "console.h"
#include "loopback.h"

/* ------------------------------------------------------------- defaults -- */
/* R4 defaults. N stays at MENU_N_FIXED (1000), because a qualification is
 * always full-count.                                                       */
static menu_settings_t settings = { 1000u, 50u, MENU_N_FIXED };

const menu_settings_t *menu_settings(void) { return &settings; }

static void show_settings(void)
{
    const menu_settings_t *s = &settings;
    printf("E %lu  T %lu  N %lu (fixed)  window [%lu, %lu]%s\n",
           (unsigned long)s->E, (unsigned long)s->T,
           (unsigned long)s->N, (unsigned long)(s->E - s->T),
           (unsigned long)(s->E + s->T),
           loopback_running() ? "  loopback ON" : "");
}

/* Strict numeric parsing (shell plumbing): a trailing non-digit makes the
 * entry malformed. The shell rejects it, and does not truncate it without
 * a message.                                                              */
static bool parse_u32(const char *s, uint32_t *out)
{
    char *end;
    unsigned long v = strtoul(s, &end, 10);
    if (end == s || *end != '\0' || v > 0xFFFFFFFFul) {
        return false;
    }
    *out = (uint32_t)v;
    return true;
}

/* ========================================================================
 * ==== STUDENT SECTION 1 — the R4 validators =============================
 * ========================================================================
 * Spec R4 is the contract:
 *   - 100 <= E <= 2,000 us (the 500 Hz to 10 kHz band)
 *   - 10 <= T <= 500 us
 *   - T < E
 *   - N is fixed at 1000. The shell gives no command that changes it.
 * Your code SHALL reject all out-of-range or malformed entries with an
 * EXPLANATORY message. Give the rule, not only the word "no". Demo D3
 * gives illegal values on purpose.
 *
 * Each validator returns true to accept the value. The shell then stores it
 * and echoes the settings again. Each validator returns false to reject the
 * value, and your message prints first. The stubs below reject all values
 * and print a loud message, so the unchanged starter cannot look like a
 * working instrument.
 * ------------------------------------------------------------------------ */

static bool validate_e(uint32_t v, const menu_settings_t *s)
{
    (void)v; (void)s;
    /* STUDENT TODO (R4): range check, the T < E interaction, and the
     * explanatory messages that reject an entry.                         */
    printf("rejected: E validator not implemented (your R4 work)\n");
    return false;
}

static bool validate_t(uint32_t v, const menu_settings_t *s)
{
    (void)v; (void)s;
    /* STUDENT TODO (R4). */
    printf("rejected: T validator not implemented (your R4 work)\n");
    return false;
}

/* ========================================================================
 * ==== STUDENT SECTION 2 — the run hook ==================================
 * ========================================================================
 * on_run(): your instrument. Capture N intervals (R2), bin each one, and
 * tally the out-of-window intervals (R5). Print the histogram and the
 * TOTAL (R5), then the statistics (R6/R7). Set the verdict LED (R8), and
 * obey the display policy and the deferred-I/O rules (R9/R10). The shell
 * echoes the window before it calls this hook.
 * ------------------------------------------------------------------------ */

static void on_run(const menu_settings_t *s)
{
    (void)s;
    /* STUDENT TODO (R2/R5-R10/R12).                                      */
    printf("run: capture engine not implemented - see "
           "Docs/Timer_Setup_for_Capture (your R2 work)\n");
}

/* ==================== END STUDENT SECTIONS ============================== */

/* ------------------------------------------------------- shell plumbing -- */

static void cmd_loop(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "on") == 0) {
        uint32_t period = 1000u;
        if (argc >= 3 && !parse_u32(argv[2], &period)) {
            printf("rejected: '%s' is not a number\n", argv[2]);
            return;
        }
        if (loopback_start(period)) {
            printf("loopback ON: %lu us square on PD12 (Arduino D2) - "
                   "jumper D2 to D5\n", (unsigned long)period);
        } else {
            printf("rejected: period must be 200..500000 us\n");
        }
        return;
    }
    if (argc >= 2 && strcmp(argv[1], "off") == 0) {
        loopback_stop();
        printf("loopback OFF\n");
        return;
    }
    printf("usage: loop on [period_us] | loop off\n");
}

static void help(void)
{
    printf("commands:\n");
    printf("  show               settings and window\n");
    printf("  e <us>             expected interval (R4 limits)\n");
    printf("  t <us>             tolerance (R4 limits, T < E)\n");
    printf("  run                capture N = 1000 intervals and report\n");
    printf("  loop on|off        LOOPBACK stimulus on D2 (provided)\n");
    printf("  help               this text\n");
}

void menu_loop(void)
{
    printf("type 'help' for commands\n\n");
    show_settings();

    for (;;) {                            /* R11: repeatable forever      */
        char  line[80];
        char *argv[8];

        printf("PC-1000> ");
        (void)console_read_line(line, sizeof line);

        int argc = console_tokenize(line, argv, 8);
        if (argc == 0) {
            continue;
        }

        uint32_t v;
        if (strcmp(argv[0], "show") == 0) {
            show_settings();
        } else if (strcmp(argv[0], "e") == 0) {
            if (argc < 2 || !parse_u32(argv[1], &v)) {
                printf("rejected: usage 'e <microseconds>' (digits only)\n");
            } else if (validate_e(v, &settings)) {
                settings.E = v;
                show_settings();
            }
        } else if (strcmp(argv[0], "t") == 0) {
            if (argc < 2 || !parse_u32(argv[1], &v)) {
                printf("rejected: usage 't <microseconds>' (digits only)\n");
            } else if (validate_t(v, &settings)) {
                settings.T = v;
                show_settings();
            }
        } else if (strcmp(argv[0], "run") == 0) {
            const menu_settings_t *s = &settings;
            /* R4: the shell echoes the active window before each run.    */
            printf("CAL-VERIFY run: window [%lu, %lu] us, N = %lu\n",
                   (unsigned long)(s->E - s->T),
                   (unsigned long)(s->E + s->T), (unsigned long)s->N);
            on_run(s);
        } else if (strcmp(argv[0], "loop") == 0) {
            cmd_loop(argc, argv);
        } else if (strcmp(argv[0], "help") == 0) {
            help();
        } else {
            printf("rejected: unknown command '%s' - type 'help'\n",
                   argv[0]);
        }
    }
}
