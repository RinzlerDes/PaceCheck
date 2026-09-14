# -----------------------------------------------------------------------------
# PC-1000 "PaceCheck" Project 1 starter — SWEN 563 / CMPE 663 / EEEE 663
# CMake project for the STM32Cube VS Code extension (GCC, Ninja).
#
# This is the file you own: add your own source files to sources_SRCS below.
# The course-provided source list, include paths, symbols, CPU flags, and
# linker script live in cmake/vscode_generated.cmake — leave that file alone.
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)

# Core project settings
project("P1_PaceCheck_Starter")
enable_language(C ASM)
message("Build type: " ${CMAKE_BUILD_TYPE})

# The CMSIS_D*/CMSIS_T* cache entries in CMakePresets.json carry the device
# identity (STM32WB5MMGHx) for the STM32Cube VS Code extension; the build
# itself does not read them. Reference them once so CMake does not warn.
set(ignore_CUBE_DEVICE_IDENTITY
    "${CMSIS_Dvendor}${CMSIS_Dname}${CMSIS_Dcore}${CMSIS_Dtz}${CMSIS_Dfpu}"
    "${CMSIS_Ddsp}${CMSIS_Dmve}${CMSIS_Dmpu}${CMSIS_Dendian}${CMSIS_Dsecure}${CMSIS_Tcompiler}")

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Core MCU flags, CPU type, instruction set and FPU setup
set(cpu_PARAMS)

# Sources — add your application .c files here
# (loopback.c is a provided module; menu.c is the shell you
#  complete — see the fenced STUDENT sections in each.)
set(sources_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/loopback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/menu.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/capture_engine.c
)

# Include directories for all compilers
set(include_DIRS)

# Include directories for each compiler
set(include_c_DIRS)
set(include_cxx_DIRS)
set(include_asm_DIRS)

# Symbols definition for all compilers
set(symbols_SYMB)

# Symbols definition for each compiler
set(symbols_c_SYMB)
set(symbols_cxx_SYMB)
set(symbols_asm_SYMB)

# Link directories and names of libraries
set(link_DIRS)
set(link_LIBS)

# Linker script
set(linker_script_SRC)

# Compiler options
set(compiler_OPTS)

# Linker options
set(linker_OPTS)

# Now call generated cmake
# This will add script generated
# information to the project
include("cmake/vscode_generated.cmake")

# Link directories setup
# Must be before executable is added
link_directories(${CMAKE_PROJECT_NAME} ${link_DIRS})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PUBLIC ${sources_SRCS})

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${include_DIRS}
    $<$<COMPILE_LANGUAGE:C>: ${include_c_DIRS}>
    $<$<COMPILE_LANGUAGE:ASM>: ${include_asm_DIRS}>
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    ${symbols_SYMB}
    $<$<COMPILE_LANGUAGE:C>: ${symbols_c_SYMB}>
    $<$<COMPILE_LANGUAGE:ASM>: ${symbols_asm_SYMB}>

    # Configuration specific
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>: >
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} ${link_LIBS})

# Compiler options
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    ${cpu_PARAMS}
    ${compiler_OPTS}
    -ffunction-sections
    -fdata-sections
    -fstack-usage
    -Wall
    -Wextra
    $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp -MMD -MP>
    $<$<CONFIG:Debug>:-O0 -g3 -ggdb>
    $<$<CONFIG:Release>:-Os>
)

# Linker options
# (newlib-nano; syscalls.c/sysmem.c provide the OS stubs. No float printf —
#  the measurement and reporting path is integer-only by spec §5.)
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -T${linker_script_SRC}
    ${cpu_PARAMS}
    ${linker_OPTS}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    --specs=nano.specs
    -Wl,--gc-sections
    -Wl,--start-group
    -lc
    -lm
    -Wl,--end-group
    -Wl,--print-memory-usage
)

# Set linker script as a dependency to force re-link if it changes
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_DEPENDS ${linker_script_SRC})

# Execute post-build to print size, generate hex and bin
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
)
