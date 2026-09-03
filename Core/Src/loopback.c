/**
 ******************************************************************************
 * @file    loopback.c
 * @brief   LOOPBACK self-stimulus on PD12 (SysTick-paced).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED MODULE — do not send it as your own work. Its ~50%
 *          duty cycle is the at-home reference for the 663 duty-cycle
 *          extension (spec section 8).
 ******************************************************************************
 */
#include "loopback.h"
#include "stm32wbxx.h"

#define SYSCLK_HZ     64000000u
#define HALF_MIN_US   100u        /* period >= 200 us                     */
#define HALF_MAX_US   250000u     /* period <= 500 ms (24-bit SysTick)    */

static volatile bool running;
static bool          phase_high;
static uint32_t      half_us;     /* configured half period               */

static void pd12_write(bool high)
{
    /* BSRR: atomic set/reset — no read-modify-write hazard, no shared
     * register touched.                                                  */
    GPIOD->BSRR = high ? GPIO_BSRR_BS12 : GPIO_BSRR_BR12;
}

bool loopback_start(uint32_t period_us)
{
    uint32_t half = period_us / 2u;
    if (half < HALF_MIN_US || half > HALF_MAX_US) {
        return false;
    }
    half_us = half;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
    (void)RCC->AHB2ENR;
    GPIOD->MODER = (GPIOD->MODER & ~GPIO_MODER_MODE12_Msk)
                 | (0x1u << GPIO_MODER_MODE12_Pos);          /* output    */
    pd12_write(false);
    phase_high = false;

    /* One SysTick interrupt in each half period, processor clock.        */
    SysTick->CTRL = 0u;
    SysTick->LOAD = (SYSCLK_HZ / 1000000u) * half - 1u;
    SysTick->VAL  = 0u;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    running = true;
    return true;
}

void loopback_stop(void)
{
    SysTick->CTRL = 0u;
    pd12_write(false);
    running = false;
}

bool loopback_running(void)
{
    return running;
}

void SysTick_Handler(void)
{
    (void)half_us;                   /* kept for future extensions        */
    if (!phase_high) {
        pd12_write(true);
        phase_high = true;
    } else {
        pd12_write(false);
        phase_high = false;
    }
}
