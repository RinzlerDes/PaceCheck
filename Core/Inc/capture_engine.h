#include <stdbool.h>
#include <stdint.h>

extern volatile bool my_flag;
extern volatile uint32_t  my_number;
extern volatile bool measurement_complete;

void capture_engine_init();
