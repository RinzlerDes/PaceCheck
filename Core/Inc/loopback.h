/**
 ******************************************************************************
 * @file    loopback.h
 * @brief   LOOPBACK self-stimulus (spec section 6): a SysTick-paced square
 *          wave on Arduino D2 = PD12, jumpered to D5 (SENSE). It tests the
 *          TIM2 capture path with no bench equipment.
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED MODULE — it tests your capture engine, not you.
 *
 *  By design the provided drivers keep SysTick free for student code, and
 *  this module owns it. PD12 has no conflict with any provided-driver pin:
 *  it is on port D, and no provided driver touches port D.
 *
 *  663 duty-cycle extension (spec section 8): the square wave of this
 *  module has a ~50% duty cycle, so it is a known at-home reference for
 *  your duty-cycle histogram. The extension itself (both-edge capture and
 *  the duty report) is student-written work, and demo row DG shows it.
 ******************************************************************************
 */
#ifndef LOOPBACK_H
#define LOOPBACK_H

#include <stdbool.h>
#include <stdint.h>

/** Start the stimulus: square wave with the given period (200 us..500 ms). */
bool loopback_start(uint32_t period_us);

/** Stop the stimulus. PD12 goes low and SysTick stops. */
void loopback_stop(void);

/** Returns true while the stimulus runs. */
bool loopback_running(void);

#endif /* LOOPBACK_H */
