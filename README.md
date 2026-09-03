# PC-1000 "PaceCheck" — Project 1 Starter Code

SWEN 563 / CMPE 663 / EEEE 663 — Real-Time & Embedded Systems, Fall 2026
Target: **STM32WB5MM-DK**, bare-metal register-level C (no HAL).

This is the starter package that the [Project 1 spec](../RTES_Project1_Spec.md) (§6) promises. It is a
complete, buildable CMake project for **VS Code with the STM32Cube extension**.

Flash it unmodified, and you get a console banner, text on the OLED, a
color-cycling RGB LED, and a 1 kHz LOOPBACK square wave on Arduino D2.
This output is proof that your board, toolchain, and terminal work before
you write firmware.

> **Graded, for each person, before the C exam (spec §10 item 0):** each team member individually builds the unmodified starter, flashes it, and opens PuTTY at their own bench.
> The check takes 10 minutes, and a TA records it. It is due before S5 (Sep 9).
> The provided `loopback.c`, with a jumper from D2 to D5, is also the capture-path self-test.
> The check is part of the D0 setup grade.

One page to keep at the bench: [`Docs/QuickRef.md`](Docs/QuickRef.md).
It gives the pin facts, the R4 limits and defaults, the demo-row index,
the generator checklist, and the two traps.

## Open and build (VS Code + STM32Cube extension)

1. Install VS Code and the **STM32Cube for Visual Studio Code** extension
   (`stmicroelectronics.stm32-vscode-extension`). At the first use, the
   extension downloads its own tool bundles (arm-none-eabi GCC, CMake,
   Ninja, ST-LINK GDB server, STM32CubeProgrammer). You do not install a
   toolchain.

2. Use **File → Open Folder…** and select this folder (`P1_PaceCheck_Starter`). When
   the extension asks to *configure the discovered CMake project as an
   STM32CubeIDE project*, accept. The extension reads the device identity
   (STM32WB5MMGHx) from `CMakePresets.json` automatically.

3. When the extension asks, select the **Debug** preset (or use the
   status bar), and build.

4. Connect the **USB STLK** port of the board and push **F5**. The
   provided launch configuration (`STM32Cube: Launch ST-Link GDB Server`)
   builds, flashes, and stops at `main`.

5. Open PuTTY on the ST-LINK virtual COM port, **115200 8N1**, and enable
   *Session → Logging → All session output*. Your PuTTY-log is demo
   evidence (R5).

Command-line equivalent (any shell with the bundle tools on `PATH`):

```
cmake --preset Debug
cmake --build --preset Debug
STM32_Programmer_CLI -c port=SWD mode=UR -w build/Debug/P1_PaceCheck_Starter.elf -v -rst
```

## What is provided (and what is yours)

The starter has three classes of files, not two:

| Provided (black boxes — do **not** modify or send) | Yours |
|---|---|
| `clock.c/h` — 64 MHz SYSCLK from the HSE crystal. TIM2 kernel clock = 64 MHz | `main.c` — replace it fully |
| `console.c/h` — USART1 VCP 115200 8N1, `printf` retarget, line-read + tokenize | all RCC/GPIO/TIM2 configuration for PB10 (SENSE) |
| `oled.c/h` — 8×21 text API for the 0.96" SSD1315 OLED on SPI1 | the capture engine, histogram, statistics — **no shell, no helper: R2 is fully yours** |
| `rgb_led.c/h` — `rgb_set(r,g,b)` for the TLC59731 single-wire LED | the fenced STUDENT sections in `menu.c` (below) |
| `loopback.c/h` — the LOOPBACK self-stimulus: SysTick square wave on PD12 (D2→D5 jumper), ~50% duty. It is the at-home reference for the 663 duty-cycle extension (§8) | |
| `Docs/Timer_Setup_for_Capture.md` (+ PDF) — TIM2 CH3 input-capture walkthrough | |
| `Docs/QuickRef.md` — the one-page bench card | |
| `syscalls.c` / `sysmem.c` / `system_stm32wbxx.c` / startup / linker script — build plumbing | |

**Shells you complete and send** (the third class): these shells have
provided plumbing, fenced STUDENT sections, and safe stubs. Each shell
marks the provided part and your part:

| Shell | Provided | Yours (fenced) |
|---|---|---|
| `menu.c/h` — CONFIG console | prompt, settings storage + R4 defaults, strict numeric parsing, window echo, dispatch loop (R11) | the R4 validators (range rules, `T < E`, every rejection message) and the run hook |

In the unmodified starter, all stubs return a safe default and announce
themselves on the console. The starter cannot hang, and it cannot get
demo-row credit for you.

## Pin ownership (spec §7.3) — read this twice

The provided drivers **exclusively own**:

```
PA1  OLED SPI1 SCK              PH0  OLED chip select
PA7  OLED SPI1 MOSI + LED data  PH1  RGB LED select
PC8  OLED reset                 PC9  OLED data/command
PB6  console USART1 TX          PB7  console USART1 RX
```

Do not reconfigure these pins. Do not write a full GPIO register on
ports A, B, C, or H. A wholesale write such as `GPIOA->MODER = ...` stops
the display during the demo.

Configure only the bits you own: `|=`, `&= ~`, masked read-modify-write.
Your pin: **PB10** (SENSE, TIM2_CH3 AF1 — with the mandatory internal
pull-down, §7.1).

The drivers are register-level C, and all register accesses use per-bit
read-modify-write. They give worked examples of the discipline that the
spec demands of you. We recommend that you read them. Do not copy them
into your report as your own work.

## Driver quick reference

```c
clock_init();                         /* first line of main()            */
clock_sysclk_hz();                    /* 64000000                        */
clock_delay_ms(10);                   /* blocking; never in capture path */

console_init();
printf("E=%lu T=%lu\n", e, t);        /* '\n' -> "\r\n"; no %f (by design) */
int ch = console_poll();              /* -1 if no key waiting            */
console_read_line(buf, sizeof buf);   /* echo, backspace, Enter          */
int n = console_tokenize(buf, argv, 8);

oled_init();
oled_clear();
oled_write_line(0, "ACQUIRING");      /* rows 0..7, 21 columns           */
oled_printf(3, "n=%4lu  %lu us", i, dt);

rgb_init();
rgb_set(0, 0, 32);                    /* blue=acquiring; green PASS; red FAIL
                                         ~11 ms per call, main loop only */
```

Design around these timing costs (R9/R10):

- one OLED line write ≈ 200 µs
- one `rgb_set()` ≈ 11 ms
- one 80-char `printf` at 115200 ≈ 7 ms

Do not put one of these calls between the first and the last captured
edge of an acquisition. This constraint is the cause of the static
`ACQUIRING` display policy (R9).

All provided drivers are **main-loop only** (not ISR-safe). Call
`clock_init()` before you use them.

## Layout

```
P1_PaceCheck_Starter/
├── CMakeLists.txt                the file you own — add your .c files here
├── CMakePresets.json             Debug/Release presets + device identity
│                                 (STM32WB5MMGHx — the extension reads this)
├── cmake/
│   ├── gcc-arm-none-eabi.cmake   arm-none-eabi toolchain file
│   └── vscode_generated.cmake    course-owned source list & flags — leave alone
├── .vscode/                      extension recommendation + F5 debug launch
├── STM32WB5MMGHX_FLASH.ld        linker script (1 MB flash / 192 KB RAM1)
├── Core/
│   ├── Inc/                      clock.h console.h oled.h rgb_led.h
│   │                             loopback.h menu.h
│   ├── Src/                      drivers + loopback.c + the menu shell
│   │                             + main.c + newlib glue
│   └── Startup/                  vector table / reset handler (ST)
├── Drivers/CMSIS/                register-definition headers (ST/ARM)
└── Docs/                         Timer Setup for Capture walkthrough
                                  + QuickRef.md (the bench card)
```

`stm32wb5mxx.h` is the module variant of `stm32wb55xx.h`. The register
structs are the same. Include `stm32wbxx.h`, and the build selects the
correct one.

## At-home testing (no bench needed)

The provided `loopback.c` puts a square wave on Arduino **D2 = PD12**
(SysTick-paced, ~50% duty). Jumper it to **D5**, and your real TIM2
capture path has a signal source with no bench equipment.

The starter `main.c` starts the wave at 1 kHz. The `loop on [period_us]` and `loop off` commands in the menu shell control it.
LOOPBACK validates your capture engine. It does not validate you.

SysTick belongs to `loopback.c`, and the other provided drivers do not
touch it. (663: the ~50% duty of LOOPBACK is the known reference for your
duty-cycle extension, §8.)

## Licenses

Course-authored files (drivers, `main.c`, docs) are for use in this
course. The files in `Drivers/CMSIS/`, the startup file, the linker
script, and `system_stm32wbxx.c` are components from STMicroelectronics
and Arm Limited. The license headers in those files (Apache-2.0 /
BSD-3-Clause) apply.
