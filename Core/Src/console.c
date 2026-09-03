/**
 ******************************************************************************
 * @file    console.c
 * @brief   USART1 virtual COM port console, 115200 8N1 (provided driver).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED DRIVER — do not change it, and do not send it as your
 *          own work.
 *
 *  Hardware: USART1 TX = PB6 (AF7), RX = PB7 (AF7), routed to the on-board
 *  ST-LINK virtual COM port. Kernel clock = PCLK2 = 64 MHz.
 *
 *  Pin discipline: this driver configures ONLY bits 6 and 7 of the GPIOB
 *  registers (read-modify-write). It does not write a full MODER, AFR or
 *  PUPDR register. Student code must obey the same rule on all shared ports.
 ******************************************************************************
 */
#include <stdio.h>
#include "stm32wbxx.h"
#include "clock.h"
#include "console.h"

#define CONSOLE_BAUD 115200u

void console_init(void)
{
    /* Enable the GPIOB and USART1 clocks (RMW keeps the other enable bits). */
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB2ENR  |= RCC_APB2ENR_USART1EN;
    (void)RCC->APB2ENR;   /* read-back: the clock starts before the writes */

    /* PB6 and PB7 -> AF7 (USART1), with a pull-up on RX so a disconnected
     * terminal reads as an idle (mark) line.
     * This code writes the bits of pins 6 and 7 only.
     */
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7))
                   | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;      /* AF   */
    GPIOB->AFR[0] = (GPIOB->AFR[0]
                     & ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7))
                    | (7u << GPIO_AFRL_AFSEL6_Pos)
                    | (7u << GPIO_AFRL_AFSEL7_Pos);                /* AF7  */
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7))
                   | GPIO_PUPDR_PUPD7_0;                           /* RX PU */

    /* 115200 8N1, oversampling by 16: BRR = round(64 MHz / 115200) = 556. */
    USART1->CR1 = 0u;                            /* known state, UE = 0     */
    USART1->BRR = (clock_sysclk_hz() + CONSOLE_BAUD / 2u) / CONSOLE_BAUD;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE;   /* 8 data, no parity       */
    USART1->CR2 = 0u;                            /* 1 stop bit              */
    USART1->CR3 = 0u;
    USART1->CR1 |= USART_CR1_UE;

    /* printf must not buffer, so the operator prompts show immediately. */
    setvbuf(stdout, NULL, _IONBF, 0);
}

void console_putc(char c)
{
    if (c == '\n') {
        while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0u) { }
        USART1->TDR = '\r';
    }
    while ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0u) { }
    USART1->TDR = (uint8_t)c;
}

void console_write(const char *s)
{
    while (*s != '\0') {
        console_putc(*s++);
    }
}

char console_getc(void)
{
    int c;
    while ((c = console_poll()) < 0) { }
    return (char)c;
}

int console_poll(void)
{
    /* A receiver overrun (a key press during a long print) must not block the
     * console. Clear ORE and continue, because instrument firmware must not
     * hang at any time.
     */
    if ((USART1->ISR & USART_ISR_ORE) != 0u) {
        USART1->ICR = USART_ICR_ORECF;
    }
    if ((USART1->ISR & USART_ISR_RXNE_RXFNE) != 0u) {
        return (int)(USART1->RDR & 0xFFu);
    }
    return -1;
}

int console_read_line(char *buf, size_t maxlen)
{
    size_t n = 0u;

    for (;;) {
        char c = console_getc();

        if (c == '\r' || c == '\n') {
            console_write("\n");
            break;
        }
        if (c == '\b' || c == 0x7F) {            /* backspace / DEL */
            if (n > 0u) {
                n--;
                console_write("\b \b");
            }
            continue;
        }
        if (c < 0x20 || c > 0x7E) {              /* ignore other controls */
            continue;
        }
        if (maxlen != 0u && n < maxlen - 1u) {
            buf[n++] = c;
            console_putc(c);
        }
    }

    if (maxlen != 0u) {
        buf[n] = '\0';
    }
    return (int)n;
}

int console_tokenize(char *line, char *argv[], int max_tokens)
{
    int argc = 0;

    while (argc < max_tokens) {
        while (*line == ' ' || *line == '\t') {  /* skip separators */
            *line++ = '\0';
        }
        if (*line == '\0') {
            break;
        }
        argv[argc++] = line;
        while (*line != '\0' && *line != ' ' && *line != '\t') {
            line++;
        }
    }
    return argc;
}

/* ---------------------------------------------------------------------------
 * printf/scanf retarget (newlib): syscalls.c routes _write/_read here.
 * ------------------------------------------------------------------------- */
int __io_putchar(int ch)
{
    console_putc((char)ch);
    return ch;
}

int __io_getchar(void)
{
    char c = console_getc();
    console_putc(c);                             /* echo */
    return (c == '\r') ? '\n' : c;
}
