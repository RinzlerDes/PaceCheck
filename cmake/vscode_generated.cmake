# -----------------------------------------------------------------------------
# Generated source list for the STM32Cube VS Code extension project layout.
# Course-owned build plumbing: students should not need to edit this file.
# Add new .c files of your own to CMakeLists.txt, not here.
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)

# Core MCU flags, CPU, instruction set and FPU setup
set(cpu_PARAMS ${cpu_PARAMS}
    -mthumb
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# Linker script
set(linker_script_SRC ${linker_script_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32WB5MMGHX_FLASH.ld
)

# Sources (everything except main.c, which lives in CMakeLists.txt)
set(sources_SRCS ${sources_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/console.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/oled.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/rgb_led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32wbxx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32wb5mmghx.s
)

# Include directories
set(include_c_DIRS ${include_c_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32WBxx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
)
set(include_cxx_DIRS ${include_cxx_DIRS}
)
set(include_asm_DIRS ${include_asm_DIRS}
)

# Symbols definition
set(symbols_c_SYMB ${symbols_c_SYMB}
    STM32WB5Mxx
    HSE_VALUE=32000000
    HSE_STARTUP_TIMEOUT=100
    HSI_VALUE=16000000
    LSE_VALUE=32768
    LSE_STARTUP_TIMEOUT=5000
    LSI_VALUE=32000
    VDD_VALUE=3300
)
set(symbols_cxx_SYMB ${symbols_cxx_SYMB}
)
set(symbols_asm_SYMB ${symbols_asm_SYMB}
)

# Link directories
set(link_DIRS ${link_DIRS}
)

# Link libraries
set(link_LIBS ${link_LIBS}
)

# Compiler options
set(compiler_OPTS ${compiler_OPTS})

# Linker options
set(linker_OPTS ${linker_OPTS})
