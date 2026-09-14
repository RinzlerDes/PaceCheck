#include <stdbool.h>
#include <stdio.h>

#include "stm32wb5mxx.h"

volatile bool my_flag = false;
volatile uint32_t  my_number = 0;
volatile uint32_t  my_delta = 0;

void capture_engine_init() {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    // RCC->AHB2ENR;

    GPIOB->MODER &= ~GPIO_MODER_MODE10_Msk;
    GPIOB->MODER |= (2u << GPIO_MODER_MODE10_Pos);

    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD10_Msk;
    GPIOB->PUPDR |= (2u << GPIO_PUPDR_PUPD10_Pos);

    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN_Msk;

    GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL10_Msk;
    GPIOB->AFR[1] |= (1u << GPIO_AFRH_AFSEL10_Pos);

    TIM2->CCMR2 &= ~TIM_CCMR2_CC3S_Msk;
    TIM2->CCMR2 |= (1u << TIM_CCMR2_CC3S_Pos);

    TIM2->CCMR2 &= ~TIM_CCMR2_IC3F_Msk;

    TIM2->CCER &= ~(TIM_CCER_CC3P_Msk | TIM_CCER_CC3NP_Msk);

    TIM2->CCMR2 &= ~TIM_CCMR2_IC3PSC_Msk;

    TIM2->CCER |= (1u << TIM_CCER_CC3E_Pos);

    TIM2->DIER |= (1u << TIM_DIER_CC3IE_Pos);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->PSC = 63;
    TIM2->ARR = 0xffffffff;
    TIM2->EGR = TIM_EGR_UG;

    TIM2->CR1 |= TIM_CR1_CEN_Msk;
}

void TIM2_IRQHandler() {
    // static uint32_t delta = 0;
    if (TIM2->SR & TIM_SR_CC3IF_Msk) {
        static uint32_t prev = 0;

        my_number = TIM2->CCR3;
        my_delta = my_number - prev;
        my_flag = true;

        prev = my_number;
    }
}