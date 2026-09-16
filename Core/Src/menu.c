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
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "capture_engine.h"
#include "console.h"
#include "loopback.h"
#include "oled.h"
#include "rgb_led.h"

/* ------------------------------------------------------------- defaults -- */
/* R4 defaults. N stays at MENU_N_FIXED (1000), because a qualification is
 * always full-count.                                                       */
static menu_settings_t settings = { 1000u, 50u, MENU_N_FIXED };

const menu_settings_t* menu_settings(void) { return &settings; }

static void show_settings(void) {
    const menu_settings_t* s = &settings;
    printf("E %lu  T %lu  N %lu (fixed)  window [%lu, %lu]%s\n",
           (unsigned long)s->E,
           (unsigned long)s->T,
           (unsigned long)s->N,
           (unsigned long)(s->E - s->T),
           (unsigned long)(s->E + s->T),
           loopback_running() ? "  loopback ON" : "");
}

/* Strict numeric parsing (shell plumbing): a trailing non-digit makes the
 * entry malformed. The shell rejects it, and does not truncate it without
 * a message.                                                              */
static bool parse_u32(const char* s, uint32_t* out) {
    char* end;
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

static bool validate_e(uint32_t v, const menu_settings_t* s) {
    // (void)v; (void)s;
    // /* STUDENT TODO (R4): range check, the T < E interaction, and the
    //  * explanatory messages that reject an entry.                         */
    // printf("rejected: E validator not implemented (your R4 work)\n");
    // return false;
    if (v < PERIOD_MIN) {
        printf("%lu is below period minimum: %lu\n", v, TOLERANCE_MIN);
        return false;
    } else if (v > PERIOD_MAX) {
        printf("%lu is above period maximum: %lu\n", v, TOLERANCE_MAX);
        return false;
    } else if (s->T >= v) {
        printf("Period: %lu must be greater than tolerance: %lu\n", v, s->T);
        return false;
    }

    return true;
}

static bool validate_t(uint32_t v, const menu_settings_t* s) {
    // (void)v; (void)s;
    // /* STUDENT TODO (R4). */
    // printf("rejected: T validator not implemented (your R4 work)\n");
    // return false;

    if (v < TOLERANCE_MIN) {
        printf("%lu is below tolerance minimum: %lu\n", v, TOLERANCE_MIN);
        return false;
    } else if (v > TOLERANCE_MAX) {
        printf("%lu is above tolerance maximum: %lu\n", v, TOLERANCE_MAX);
        return false;
    } else if (v >= s->E) {
        printf("Period: %lu must be greater than tolerance: %lu\n", v, s->T);
        return false;
    }
    return true;
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
static uint32_t isqrt_u64(uint64_t n) {
    uint64_t low = 0;
    uint64_t high = UINT32_MAX;
    uint64_t answer = 0;

    while (low <= high) {
        uint64_t mid = low + (high - low) / 2;

        if (mid == 0 || mid <= n / mid) {
            answer = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return (uint32_t)answer;
}

static void on_run(const menu_settings_t* s) {
    // (void)s;
    // /* STUDENT TODO (R2/R5-R10/R12).                                      */
    // printf("run: capture engine not implemented - see "
    //        "Docs/Timer_Setup_for_Capture (your R2 work)\n");
    // capture_engine_start(s);
    // while
    rgb_set(0, 0, 10);
    oled_clear();
    oled_write_line(3, "ACQUIRING");

    capture_engine_start(s);
    while (!capture_engine_complete()) {}

    if (trash_data) {
        printf("entire test is trash, period is really messed up: %lu, max %lu\n",
               trash_period,
               PERIOD_MAX);
    }

    const volatile capture_engine_result* result = capture_engine_result_get();

    // REMOVE
    // fill_dummy_data();

    bool pass = result->out_high == 0u && result->out_low == 0;
    uint32_t in_window = result->total - result->out_high - result->out_low;
    uint32_t min = BINS_SIZE;
    uint32_t max = 0u;
    uint64_t sum = 0u;
    uint64_t sum2 = 0u;
    uint32_t tolerance_lower = s->E - s->T;
    uint32_t tolerance_upper = s->E + s->T;

    printf("\n");
    for (uint32_t period = 0u; period < BINS_SIZE; period++) {
        if (period == tolerance_lower) {
            printf("Out Of Range Lower ^^^^\n");
        }

        uint32_t period_count = result->bins[period];

        if (period_count != 0u) {
            sum += (uint64_t)period * period_count;
            sum2 += (uint64_t)period * period * period_count;

            if (period < min) {
                min = period;
            } else if (period > max) {
                max = period;
            }
            printf("%lu: %lu\n", period, result->bins[period]);
        }

        if (period == tolerance_upper) {
            printf("Out Of Range Upper vvvv\n");
        }
    }
    printf("\n");

    uint64_t mean = (sum * 10u + s->N / 2) / s->N;
    uint64_t variance = 100u * (s->N * (sum2) - (sum * sum)) / (s->N * (s->N - 1));
    uint32_t sample_standard_deviation = isqrt_u64(variance);
    uint64_t rate_ppm = (10u * 60000000ull * s->N + sum / 2u) / sum;

    uint32_t worst_early = s->E - min;
    uint32_t worst_late = max - s->E;
    uint32_t worst_largest = worst_early > worst_late ? worst_early : worst_late;
    // uint32_t margin_to_limit = ;

    printf("Out Low: %lu\n", result->out_low);

    printf("Out High: %lu\n", result->out_high);

    printf("Total: %lu In Window + %lu Out Low + %lu Out High = %lu\n",
           in_window,
           result->out_low,
           result->out_high,
           result->total);

    printf("Min: %lu\n", min);
    printf("Max: %lu\n", max);
    printf("Mean: %lu.%lu\n", (unsigned long)mean / 10, mean % 10);
    printf("Sample Standard Deviation: %lu.%lu\n",
           sample_standard_deviation / 10,
           sample_standard_deviation % 10);
    printf("Rate: %lu.%lu ppm\n", (unsigned long)rate_ppm / 10u, (unsigned long)rate_ppm % 10u);
    printf("Worst Early: -%lu\n", worst_early);
    printf("Worst Late: +%lu\n", worst_late);
    if (pass) {
        printf("Margin To Limit: %lu\n", worst_largest);
    }

    char* verdict = pass ? "Pass" : "Fail";
    printf("Verdict: %s\n", verdict);
    printf("\n");

    if (pass) {
        rgb_set(0, 10, 0);
    } else {
        rgb_set(10, 0, 0);
    }

    oled_clear();
    oled_printf(0u, "%s\n", verdict);
    oled_printf(1u, "Min: %ul\n", min);
    oled_printf(2u, "Max: %ul\n", max);
}

/* ==================== END STUDENT SECTIONS ============================== */

/* ------------------------------------------------------- shell plumbing -- */

static void cmd_loop(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "on") == 0) {
        uint32_t period = 1000u;
        if (argc >= 3 && !parse_u32(argv[2], &period)) {
            printf("rejected: '%s' is not a number\n", argv[2]);
            return;
        }
        if (loopback_start(period)) {
            printf(
                "loopback ON: %lu us square on PD12 (Arduino D2) - "
                "jumper D2 to D5\n",
                (unsigned long)period);
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

static void help(void) {
    printf("commands:\n");
    printf("  show               settings and window\n");
    printf("  e <us>             expected interval (R4 limits)\n");
    printf("  t <us>             tolerance (R4 limits, T < E)\n");
    printf("  run                capture N = 1000 intervals and report\n");
    printf("  loop on|off        LOOPBACK stimulus on D2 (provided)\n");
    printf("  help               this text\n");
}

void menu_loop(void) {
    printf("type 'help' for commands\n\n");
    show_settings();

    for (;;) { /* R11: repeatable forever      */
        char line[80];
        char* argv[8];

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
            const menu_settings_t* s = &settings;
            /* R4: the shell echoes the active window before each run.    */
            printf("CAL-VERIFY run: window [%lu, %lu] us, N = %lu\n",
                   (unsigned long)(s->E - s->T),
                   (unsigned long)(s->E + s->T),
                   (unsigned long)s->N);
            on_run(s);
        } else if (strcmp(argv[0], "loop") == 0) {
            cmd_loop(argc, argv);
        } else if (strcmp(argv[0], "help") == 0) {
            help();
        } else {
            printf("rejected: unknown command '%s' - type 'help'\n", argv[0]);
        }
    }
}
