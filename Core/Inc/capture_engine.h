#include <stdbool.h>
#include <stdint.h>
#include "menu.h"

#define STARTUP_PERIOD 100000u
#define BINS_SIZE (PERIOD_MAX + TOLERANCE_MAX + 1)

typedef struct {
    uint32_t out_low;
    uint32_t out_high;
    uint32_t measurements_count;
    uint32_t bins[BINS_SIZE];
    uint32_t measurements[MENU_N_FIXED];
} capture_engine_result;

// extern volatile bool my_flag;
// extern volatile uint32_t  my_number;
extern volatile bool measurement_complete;
// extern volatile bool trash_data;
// extern volatile uint32_t trash_period;

void capture_engine_init();
bool signal_detected(uint32_t period);
void capture_engine_start(const menu_settings_t* settings);
const volatile capture_engine_result* capture_engine_result_get();
bool capture_engine_complete();
void fill_dummy_data();
