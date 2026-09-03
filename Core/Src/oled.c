/**
 ******************************************************************************
 * @file    oled.c
 * @brief   Text driver for the on-board 0.96" 128x64 OLED — SSD1315
 *          controller on SPI1 (provided driver).
 *
 *          SWEN 563 / CMPE 663 — Project 1 "PaceCheck PC-1000" starter code.
 *          PROVIDED DRIVER — do not change it, and do not send it as your
 *          own work.
 *
 *  Hardware (STM32WB5MM-DK, UM2825 / board BSP):
 *      PA1  SPI1 SCK  (AF5)         PH0  chip select (active low)
 *      PA7  SPI1 MOSI (AF5)         PC9  data/command (low = command)
 *                                   PC8  reset (active low)
 *      SPI1: 64 MHz PCLK2 / 8 = 8 MHz, mode CPOL=1/CPHA=0, MSB first,
 *      8-bit, software NSS. Write-only, because the panel has no MISO
 *      connection.
 *
 *  PA7 SHARES the TLC59731 RGB-LED data line (jumper JP5). The rgb_led
 *  driver borrows PA7 as a plain GPIO output and then gives it back. This
 *  driver re-asserts AF5 on PA1 and PA7 at each public call. Thus the MAIN
 *  LOOP can interleave the two provided drivers freely. No provided driver
 *  is interrupt-safe.
 *
 *  Pin discipline: each GPIO configuration below is a read-modify-write on
 *  the owned bits only. Ports A and C also have student-owned pins.
 ******************************************************************************
 */
#include <stdio.h>
#include <stdarg.h>
#include "stm32wbxx.h"
#include "clock.h"
#include "oled.h"

#define OLED_PAGE_BYTES 128u

static uint8_t oled_up;   /* init guard: public calls no-op until init */

/* ---------------------------------------------------------------------------
 * 5x7 font, ASCII 0x20..0x7E. Column-major, bit 0 = top pixel. Many
 * small-LCD drivers use this public-domain glyph set.
 * ------------------------------------------------------------------------- */
static const uint8_t font5x7[95][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */  {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x14,0x08,0x3E,0x08,0x14}, /* * */    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */    {0x00,0x7F,0x41,0x41,0x00}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* \ */    {0x00,0x41,0x41,0x7F,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */    {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */    {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7F,0x48,0x44,0x44,0x38}, /* b */    {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7F}, /* d */    {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7E,0x09,0x01,0x02}, /* f */    {0x0C,0x52,0x52,0x52,0x3E}, /* g */
    {0x7F,0x08,0x04,0x04,0x78}, /* h */    {0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3D,0x00}, /* j */    {0x7F,0x10,0x28,0x44,0x00}, /* k */
    {0x00,0x41,0x7F,0x40,0x00}, /* l */    {0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x7C,0x08,0x04,0x04,0x78}, /* n */    {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7C,0x14,0x14,0x14,0x08}, /* p */    {0x08,0x14,0x14,0x18,0x7C}, /* q */
    {0x7C,0x08,0x04,0x04,0x08}, /* r */    {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3F,0x44,0x40,0x20}, /* t */    {0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C}, /* v */    {0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */    {0x0C,0x50,0x50,0x50,0x3C}, /* y */
    {0x44,0x64,0x54,0x4C,0x44}, /* z */    {0x00,0x08,0x36,0x41,0x00}, /* { */
    {0x00,0x00,0x7F,0x00,0x00}, /* | */    {0x00,0x41,0x36,0x08,0x00}, /* } */
    {0x02,0x01,0x02,0x04,0x02}, /* ~ */
};

/* ---------------------------------------------------------------------------
 * Pin ownership. Each public entry calls this function again, so it repairs
 * the PA7 handoff from the rgb_led driver. It repairs nothing else.
 * ------------------------------------------------------------------------- */
static void oled_pins_assert(void)
{
    /* PA1 (SCK) / PA7 (MOSI) -> AF5, medium-speed, pull-down (an idle-low
     * data line keeps the shared LED input quiet while PH1 deselects it). */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE1 | GPIO_MODER_MODE7))
                   | GPIO_MODER_MODE1_1 | GPIO_MODER_MODE7_1;       /* AF   */
    GPIOA->AFR[0] = (GPIOA->AFR[0]
                     & ~(GPIO_AFRL_AFSEL1 | GPIO_AFRL_AFSEL7))
                    | (5u << GPIO_AFRL_AFSEL1_Pos)
                    | (5u << GPIO_AFRL_AFSEL7_Pos);                 /* AF5  */
    GPIOA->OSPEEDR = (GPIOA->OSPEEDR
                      & ~(GPIO_OSPEEDR_OSPEED1 | GPIO_OSPEEDR_OSPEED7))
                     | GPIO_OSPEEDR_OSPEED1_0 | GPIO_OSPEEDR_OSPEED7_0;
    GPIOA->PUPDR = (GPIOA->PUPDR & ~(GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD7))
                   | GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD7_1;       /* PD   */

    /* PH0 (CS), PC8 (RST), PC9 (D/C) -> push-pull outputs. */
    GPIOH->MODER = (GPIOH->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_0;
    GPIOC->MODER = (GPIOC->MODER & ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9))
                   | GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0;
}

/* CS / D/C / RST through BSRR — atomic, no read-modify-write race on ODR. */
static inline void cs_low(void)   { GPIOH->BSRR = GPIO_BSRR_BR0; }
static inline void cs_high(void)  { GPIOH->BSRR = GPIO_BSRR_BS0; }
static inline void dc_low(void)   { GPIOC->BSRR = GPIO_BSRR_BR9; }
static inline void dc_high(void)  { GPIOC->BSRR = GPIO_BSRR_BS9; }
static inline void rst_low(void)  { GPIOC->BSRR = GPIO_BSRR_BR8; }
static inline void rst_high(void) { GPIOC->BSRR = GPIO_BSRR_BS8; }

/* ---------------------------------------------------------------------------
 * SPI1, transmit-only. This code accesses DR as a byte, because a 32-bit
 * write pushes two frames into the TX FIFO (RM0434, "data packing").
 * ------------------------------------------------------------------------- */
static void spi_tx(uint8_t b)
{
    while ((SPI1->SR & SPI_SR_TXE) == 0u) { }
    *(volatile uint8_t *)&SPI1->DR = b;
    while ((SPI1->SR & SPI_SR_RXNE) == 0u) { }   /* frame fully shifted out */
    (void)*(volatile uint8_t *)&SPI1->DR;        /* discard dummy RX        */
}

static void oled_cmd(uint8_t c)
{
    cs_low();
    dc_low();
    spi_tx(c);
    cs_high();
}

static void oled_data(const uint8_t *p, uint32_t len)
{
    cs_low();
    dc_high();
    while (len-- > 0u) {
        spi_tx(*p++);
    }
    dc_low();
    cs_high();
}

/* Address one text row: page window = this page, column window = 0..127. */
static void oled_set_row_window(uint8_t row)
{
    oled_cmd(0x22u);            /* set page address range   */
    oled_cmd(row);
    oled_cmd(row);
    oled_cmd(0x21u);            /* set column address range */
    oled_cmd(0x00u);
    oled_cmd(0x7Fu);
}

void oled_init(void)
{
    /* Clocks for GPIOA/C/H and SPI1 (RMW on the enable registers). */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN
                    | RCC_AHB2ENR_GPIOHEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    (void)RCC->APB2ENR;

    oled_pins_assert();
    cs_high();
    dc_low();

    /* SPI1: master, 64/8 = 8 MHz, CPOL=1/CPHA=0, MSB first, software NSS. */
    SPI1->CR1 = 0u;                              /* SPE = 0 during the setup  */
    SPI1->CR2 = (7u << SPI_CR2_DS_Pos)           /* 8-bit frames              */
                | SPI_CR2_FRXTH;                 /* RXNE at 1 byte            */
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI
                | SPI_CR1_CPOL
                | (2u << SPI_CR1_BR_Pos);        /* fPCLK/8                   */
    SPI1->CR1 |= SPI_CR1_SPE;

    /* Hardware reset, then a fixed delay for the panel supply to stabilize. */
    rst_low();
    clock_delay_ms(1u);
    rst_high();
    clock_delay_ms(100u);

    /* Init sequence from the ST BSP for this module (SSD1315 datasheet):
     *   - display off
     *   - charge-pump on
     *   - horizontal addressing
     *   - start line 0
     *   - remapped scan and segment orientation, for the mounting of the DK
     *   - display on.
     * All other controls keep their power-on default values.
     */
    oled_cmd(0xAEu);            /* display off               */
    oled_cmd(0x8Du);            /* charge-pump control ...   */
    oled_cmd(0x14u);            /* ... enable during display */
    oled_cmd(0x20u);            /* memory addressing mode... */
    oled_cmd(0x00u);            /* ... horizontal            */
    oled_cmd(0x40u);            /* display start line 0      */
    oled_cmd(0xC8u);            /* COM scan: remapped        */
    oled_cmd(0xA1u);            /* segment remap             */
    oled_cmd(0xAFu);            /* display on                */

    oled_up = 1u;
    oled_clear();
}

void oled_clear(void)
{
    uint8_t zeros[OLED_PAGE_BYTES] = {0u};

    if (oled_up == 0u) {
        return;
    }
    oled_pins_assert();
    for (uint8_t row = 0u; row < OLED_ROWS; row++) {
        oled_set_row_window(row);
        oled_data(zeros, sizeof zeros);
    }
}

void oled_write_line(uint8_t row, const char *text)
{
    uint8_t line[OLED_PAGE_BYTES];
    uint32_t x = 0u;

    if (oled_up == 0u || row >= OLED_ROWS || text == NULL) {
        return;
    }
    oled_pins_assert();

    /* Write up to 21 glyphs (6 px cell: 5 font columns + 1 space column),
     * then pad to the full 128 px so the new line replaces the previous
     * content. */
    for (uint32_t col = 0u; col < OLED_COLS; col++) {
        char c = (*text != '\0') ? *text++ : ' ';
        const uint8_t *glyph = (c >= 0x20 && c <= 0x7E)
                               ? font5x7[c - 0x20] : font5x7[0];
        for (uint32_t i = 0u; i < 5u; i++) {
            line[x++] = glyph[i];
        }
        line[x++] = 0x00u;                       /* inter-glyph gap */
    }
    while (x < OLED_PAGE_BYTES) {
        line[x++] = 0x00u;                       /* 2 px right margin */
    }

    oled_set_row_window(row);
    oled_data(line, sizeof line);
}

void oled_printf(uint8_t row, const char *fmt, ...)
{
    char buf[OLED_COLS + 1u];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    oled_write_line(row, buf);
}
