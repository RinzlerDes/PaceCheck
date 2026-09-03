/**
 ******************************************************************************
 * @file    sysmem.c
 * @brief   Minimal newlib _sbrk function (the heap that the internal
 *          printf buffers use) (provided code — do not change it).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *
 *  The heap grows from _end (the linker script sets it) to the stack.
 *  A collision with the current stack pointer returns ENOMEM and does not
 *  silently corrupt memory.
 ******************************************************************************
 */
#include <errno.h>
#include <stdint.h>

extern uint8_t _end;      /* linker script: end of .bss           */
extern uint8_t _estack;   /* linker script: initial stack pointer */

static uint8_t *heap_break = &_end;

void *_sbrk(ptrdiff_t incr)
{
    uint8_t *stack_now;
    __asm volatile ("mrs %0, msp" : "=r" (stack_now));

    if (heap_break + incr > stack_now) {
        errno = ENOMEM;
        return (void *)-1;
    }

    uint8_t *prev = heap_break;
    heap_break += incr;
    return prev;
}
