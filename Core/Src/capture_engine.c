#include "capture_engine.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "stm32wb5mxx.h"

// #define MEASUREMENT_COUNT 1000u

// static volatile uint32_t bins[BINS_SIZE];

// volatile bool my_flag = false;
static volatile bool edge_detected;
static volatile bool first_edge = true;
// volatile uint32_t my_number = 0;
volatile uint32_t prev = 0;
// volatile uint32_t my_delta = 0;
// volatile uint32_t measurements[MENU_N_FIXED];
volatile bool measurement_complete = false;
// static volatile uint32_t tolerance = 0;
// static volatile uint32_t measurement_count = 0;
// static uint32_t dummy_data[1000];
// volatile uint32_t out_low_count = 0;
// volatile uint32_t out_high_count = 0;
volatile uint32_t tolerance_lower = 0;
volatile uint32_t tolerance_upper = 0;
// uint32_t measurements_per_run = 0;
// static capture_engine_result result = { .out_low_count = 0u,
//                                                  .out_high_count = 0u,
//                                                  .bins = bins };
static volatile capture_engine_result result;

static uint32_t dummy_data[1000] = {
    980, 982, 992, 986, 995, 993, 990, 986, 995, 993, 991, 994, 991, 988, 995, 992, 989, 990, 994,
    987, 987,  985, 985, 986, 989, 991, 994, 986, 992, 987, 995, 992, 990, 987, 986, 987, 994, 990,

    // low line
    // 187, 187,  185, 185, 186, 181, 111, 114, 186, 112, 187, 115, 112, 110, 187, 186, 187, 114, 110,

    992, 986,  992, 993, 993, 993, 988, 995, 995, 994, 985, 988, 989, 994, 994, 986, 987, 985, 994,
    994, 995,  989, 985, 995, 986, 990, 986, 995, 985, 994, 985, 992, 992, 992, 994, 991, 993, 987,
    991, 993,  989, 995, 995, 988, 985, 989, 989, 990, 985, 992, 990, 991, 992, 995, 985, 987, 987,
    986, 985,  985, 994, 985, 995, 991, 991, 986, 987, 995, 990, 987, 992, 987, 993, 986, 992, 986,
    994, 991,  992, 992, 991, 985, 991, 993, 987, 987, 987, 993, 988, 993, 993, 995, 992, 988, 988,
    987, 986,  988, 992, 987, 989, 992, 989, 993, 990, 990, 988, 993, 993, 990, 991, 986, 988, 985,
    989, 990,  986, 985, 986, 987, 990, 990, 995, 990, 985, 987, 991, 989, 991, 985, 992, 987, 995,
    991, 989,  993, 989, 995, 994, 993, 987, 993, 993, 986, 992, 991, 992, 990, 993, 985, 993, 987,
    986, 991,  989, 985, 988, 988, 988, 995, 986, 991, 995, 986, 985, 990, 993, 988, 990, 993, 992,
    995, 994,  993, 990, 994, 988, 993, 989, 988, 994, 992, 985, 986, 995, 988, 989, 991, 989, 986,
    986, 986,  994, 989, 991, 987, 995, 987, 988, 994, 992, 994, 995, 986, 994, 985, 992, 985, 985,
    985, 990,  988, 992, 989, 991, 995, 994, 985, 990, 985, 995, 986, 991, 988, 995, 992, 992, 992,
    985, 985,  993, 989, 994, 986, 987, 992, 988, 990, 989, 988, 990, 994, 994, 994, 995, 995, 995,
    990, 994,  994, 989, 988, 989, 991, 989, 990, 993, 990, 985, 994, 986, 987, 994, 991, 995, 989,
    995, 995,  991, 989, 993, 988, 994, 988, 990, 991, 993, 988, 992, 991, 988, 989, 991, 986, 989,
    987, 994,  987, 990, 993, 985, 995, 995, 992, 987, 989, 988, 985, 991, 991, 995, 989, 992, 993,
    988, 986,  985, 992, 985, 987, 987, 989, 995, 992, 985, 990, 992, 989, 990, 988, 985, 993, 994,
    995, 985,  994, 992, 986, 987, 988, 985, 987, 992, 990, 988, 993, 994, 991, 993, 989, 992, 986,
    992, 985,  993, 993, 985, 991, 989, 993, 988, 991, 994, 991, 985, 989, 992, 994, 987, 985, 991,
    995, 995,  988, 987, 985, 985, 985, 985, 994, 985, 989, 990, 994, 992, 991, 989, 985, 985, 988,
    992, 986,  989, 993, 992, 985, 992, 992, 990, 985, 991, 995, 995, 992, 993, 995, 992, 991, 986,
    995, 993,  992, 994, 988, 988, 985, 985, 986, 990, 986, 995, 985, 986, 985, 987, 992, 995, 989,
    989, 989,  992, 991, 990, 994, 995, 991, 988, 992, 995, 991, 993, 992, 992, 990, 986, 990, 991,
    991, 995,  991, 990, 988, 994, 994, 989, 986, 992, 990, 992, 992, 989, 994, 995, 991, 987, 987,
    993, 993,  990, 991, 994, 989, 987, 988, 990, 993, 986, 990, 994, 995, 993, 990, 985, 994, 987,
    989, 989,  986, 992, 995, 988, 987, 986, 994, 991, 995, 989, 987, 985, 994, 989, 990, 988, 985,
    988, 985,  995, 992, 988, 989, 992, 993, 985, 991, 995, 986, 991, 993, 992, 985, 988, 986, 990,
    986, 992,  988, 990, 989, 993, 991, 985, 986, 992, 989, 988, 987, 985, 995, 989, 992, 986, 991,
    995, 987,  985, 990, 987, 993, 993, 991, 995, 995, 986, 993, 989, 985, 986, 985, 989, 988, 992,
    986, 988,  991, 991, 991, 993, 995, 985, 988, 985, 989, 990, 986, 989, 993, 995, 985, 992, 991,
    994, 988,  986, 987, 989, 985, 985, 990, 995, 992, 987, 995, 989, 985, 986, 988, 986, 989, 991,
    995, 986,  992, 988, 993, 986, 990, 985, 985, 991, 994, 992, 992, 987, 989, 985, 993, 992, 987,
    989, 994,  994, 992, 986, 987, 991, 986, 994, 995, 987, 991, 991, 986, 989, 991, 989, 986, 994,
    992, 992,  993, 987, 993, 993, 991, 993, 990, 988, 995, 989, 992, 986, 992, 990, 988, 993, 995,
    993, 991,  987, 987, 987, 991, 987, 985, 986, 986, 985, 994, 989, 988, 985, 990, 991, 990, 985,
    995, 987,  988, 990, 988, 992, 988, 989, 988, 985, 989, 994, 991, 992, 991, 993, 989, 988, 992,
    986, 991,  993, 990, 985, 988, 990, 986, 987, 992, 995, 987, 989, 991, 987, 990, 988, 988, 991,
    995, 988,  987, 993, 988, 986, 993, 991, 991, 988, 985, 995, 995, 992, 992, 994, 988, 987, 989,
    991, 992,  994, 985, 991, 995, 986, 987, 995, 986, 993, 985, 986, 988, 985, 989, 990, 992, 988,
    994, 987,  988, 993, 985, 995, 993, 995, 991, 993, 989, 989, 992, 992, 994, 995, 989, 985, 987,
    986, 986,  988, 986, 995, 993, 992, 994, 985, 991, 991, 995, 991, 990, 985, 985, 985, 994, 989,
    986, 986,  992, 987, 988, 989, 986, 992, 990, 990, 995, 993, 989, 993, 993, 995, 987, 988, 992,
    990, 991,  992, 991, 990, 989, 995, 995, 994, 990, 993, 994, 986, 992, 986, 992, 988, 986, 995,
    985, 993,  988, 988, 986, 991, 995, 991, 992, 995, 987, 988, 993, 988, 991, 989, 989, 989, 991,
    986, 994,  990, 994, 987, 985, 990, 991, 989, 995, 994, 991, 989, 986, 985, 994, 992, 993, 985,
    989, 986,  990, 987, 986, 993, 991, 995, 992, 987, 987, 986, 986, 995, 986, 986, 994, 992, 995,
    985, 986,  991, 991, 990, 994, 989, 994, 989, 992, 993, 992, 993, 993, 985, 991, 991, 990, 992,
    990, 994,  995, 986, 988, 991, 994, 992, 990, 994, 987, 985, 987, 994, 993, 987, 991, 993, 987,
    995, 988,  994, 994, 991, 992, 995, 995, 993, 995, 985, 994, 992, 989, 992, 993, 990, 990, 987,
    987, 993,  991, 994, 989, 992, 986, 991, 989, 985, 990, 985, 988, 990, 985, 994, 995, 990, 988,
    987, 990,  987, 991, 995, 991, 994, 990, 986, 993, 994, 993
};

void capture_engine_init() {
    // turn on gpiob
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // set pin 10 mode 0b10 is alternate function
    GPIOB->MODER &= ~GPIO_MODER_MODE10_Msk;
    GPIOB->MODER |= (2u << GPIO_MODER_MODE10_Pos);

    // set pin 10 to pull down 0b10
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD10_Msk;
    GPIOB->PUPDR |= (2u << GPIO_PUPDR_PUPD10_Pos);

    // enable clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN_Msk;

    // clear then set alt func sel port 10 to 0b0001 which is AF1
    GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL10_Msk;
    GPIOB->AFR[1] |= (1u << GPIO_AFRH_AFSEL10_Pos);

    // in cap cmp mode reg 2 where channel 3 lives, set cap cmp selection to input
    // where input cap 3 is mapped to time input 3 which is 0b01
    TIM2->CCMR2 &= ~TIM_CCMR2_CC3S_Msk;
    TIM2->CCMR2 |= (1u << TIM_CCMR2_CC3S_Pos);

    // set inp cap filter to 0
    TIM2->CCMR2 &= ~TIM_CCMR2_IC3F_Msk;
    // TIM2->CCMR2 |= (3u << TIM_CCMR2_IC3F_Pos);

    // cap cmp channel 3 will read rising edges
    TIM2->CCER &= ~(TIM_CCER_CC3P_Msk | TIM_CCER_CC3NP_Msk);

    // no prescalar for inp cap channel 3
    TIM2->CCMR2 &= ~TIM_CCMR2_IC3PSC_Msk;

    // enable cap compare for channel 3
    TIM2->CCER |= (1u << TIM_CCER_CC3E_Pos);

    // // interrupt enable channel 3
    // TIM2->DIER |= (1u << TIM_DIER_CC3IE_Pos);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->PSC = 63u;
    // full range
    TIM2->ARR = 0xffffffff;
    // causes tim2 to use updated prescalar immediately, not necessary i guess
    TIM2->EGR = TIM_EGR_UG;

    // start timer
    TIM2->CR1 |= TIM_CR1_CEN_Msk;
}

const volatile capture_engine_result* capture_engine_result_get() { return &result; }

void tim2_channel3_disable_interrupt() { TIM2->DIER &= ~TIM_DIER_CC3IE_Msk; }

void tim2_channel3_enable_interrupt() {
    tim2_channel3_disable_interrupt();
    (void)TIM2->CCR3;
    TIM2->DIER |= (1u << TIM_DIER_CC3IE_Pos);
}

bool signal_detected(uint32_t period) {
    tim2_channel3_disable_interrupt();
    edge_detected = false;
    first_edge = true;
    uint32_t now = TIM2->CNT;
    tim2_channel3_enable_interrupt();

    while ((TIM2->CNT - now) < period) {
        if (edge_detected) {
            tim2_channel3_disable_interrupt();
            return true;
        }
    }

    // TIM2->SR &= ~TIM_SR_CC3IF_Msk;
    // uint32_t start = TIM2->CNT;

    // while((TIM2->CNT - start) < period) {
    //     if (TIM2->SR & TIM_SR_CC3IF_Msk) {
    //         TIM2->CCR3;
    //         return true;
    //     }
    // }

    tim2_channel3_disable_interrupt();
    return false;
}

void capture_engine_start(const menu_settings_t* settings) {
    // edge_detected = false;
    // measurement_complete = false;
    // memset(bins, 0u, sizeof(bins[0]) * BINS_SIZE);
    // prev = 0;
    // measurement_count = 0;
    // out_low_count = 0;
    // out_high_count = 0;
    // tolerance_lower = settings.E - settings.T;
    // tolerance_upper = settings.E + settings.T;
    // measurements_per_run = settings.N;
    tim2_channel3_disable_interrupt();

    // trash_data = false;
    first_edge = true;
    measurement_complete = false;
    memset(result.bins, 0u, sizeof(result.bins[0]) * BINS_SIZE);
    memset(result.measurements, 0u, sizeof(result.measurements[0]) * MENU_N_FIXED);
    result.out_high = 0;
    result.out_low = 0;
    result.measurements_count = 0;
    tolerance_lower = settings->E - settings->T;
    tolerance_upper = settings->E + settings->T;
    // measurements_per_run = settings->N;

    tim2_channel3_enable_interrupt();
}

void TIM2_IRQHandler() {
    // static uint32_t delta = 0;
    // if ((TIM2->SR & TIM_SR_CC3IF_Msk) == 0u || measurement_complete) {
    if ((TIM2->SR & TIM_SR_CC3IF_Msk) == 0u) {
        // TIM2->SR &= ~TIM_SR_CC3IF_Msk;
        return;
    }

    // consume/reset time/flag
    uint32_t now = TIM2->CCR3;

    if (first_edge) {
        first_edge = false;
        prev = now;
        edge_detected = true;
        return;
    }

    uint32_t period = now - prev;
    prev = now;
    edge_detected = true;

    // if (measurement_complete) {
    //     return;
    // }

    if (period < tolerance_lower) {
        result.out_low++;
    } else if (period > tolerance_upper) {
        result.out_high++;
    } else {
        result.bins[period] += 1;
    }
    // else {
    //     trash_data = true;
    //     trash_period = period;
    // }

    result.measurements[result.measurements_count] = period;
    result.measurements_count++;

    measurement_complete = result.measurements_count >= MENU_N_FIXED;

    if (measurement_complete) {
        tim2_channel3_disable_interrupt();
    }
}

bool capture_engine_complete() { return measurement_complete; }

void fill_dummy_data() {
    memset(result.measurements, 0u, sizeof(result.measurements[0]) * MENU_N_FIXED);
    memset(result.bins, 0u, sizeof(result.bins[0]) * BINS_SIZE);
    result.out_low = 0;
    result.out_high = 0;

    for (uint32_t i = 0; i < 1000; i++) {
        uint32_t period = dummy_data[i];
        result.measurements[i] = period;

        if (period < tolerance_lower) {
            result.out_low++;
        } else if (period > tolerance_upper) {
            result.out_high++;
        } else {
            result.bins[period] += 1;
        }
    }
}