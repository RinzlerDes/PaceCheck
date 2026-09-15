#include <stdbool.h>
#include <stdio.h>

#include "stm32wb5mxx.h"

#define MEASUREMENT_COUNT 1000u

volatile bool my_flag = false;
volatile uint32_t my_number = 0;
volatile uint32_t prev = 0;
volatile uint32_t my_delta = 0;
volatile uint32_t measurements[MEASUREMENT_COUNT];
volatile bool measurement_complete = false;

static volatile uint32_t tolerance = 0;
static volatile uint32_t measurement_count = 0;

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

    // interrupt enable channel 3
    TIM2->DIER |= (1u << TIM_DIER_CC3IE_Pos);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->PSC = 63u;
    // full range
    TIM2->ARR = 0xffffffff;
    // causes tim2 to use updated prescalar immediately, not necessary i guess
    TIM2->EGR = TIM_EGR_UG;

    // start timer
    TIM2->CR1 |= TIM_CR1_CEN_Msk;
}

void TIM2_IRQHandler() {
    // static uint32_t delta = 0;
    // if ((TIM2->SR & TIM_SR_CC3IF_Msk) == 0u || measurement_complete) {
    if ((TIM2->SR & TIM_SR_CC3IF_Msk) == 0u) {
        // TIM2->SR &= ~TIM_SR_CC3IF_Msk;
        return;
    }

    my_number = TIM2->CCR3;
    my_flag = true;

    if (measurement_complete) {
        return;
    }

    my_delta = my_number - prev;

    prev = my_number;

    measurements[measurement_count] = my_delta;
    measurement_count++;
    measurement_complete = measurement_count >= MEASUREMENT_COUNT;
}