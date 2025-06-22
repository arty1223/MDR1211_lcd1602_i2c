cmake_minimum_required(VERSION 3.20)

enable_language(C ASM)
project(milandr_sdk)
add_library(milandr_sdk INTERFACE)

target_include_directories(milandr_sdk INTERFACE
    "CMSIS/CoreSupport"
    "CMSIS/DeviceSupport/inc"
    "CMSIS/DeviceSupport/startup"
    "SPL/inc"
    "SPL"      
)

target_sources(milandr_sdk INTERFACE
    "../Startup/startup_gcc_MDR32F9Q2I.s"
    "CMSIS/DeviceSupport/startup/system_MDR32F9Q2I.c"
    "SPL/src/MDR32FxQI_asm_GCC.S"
    "SPL/src/syscalls.c"
    "SPL/src/MDR32FxQI_rst_clk.c"
    "SPL/src/MDR32FxQI_port.c"
    "SPL/src/MDR32FxQI_eeprom.c"
    "SPL/src/MDR32FxQI_i2c.c"
    "SPL/src/MDR32FxQI_bkp.c"   
)

target_link_directories(milandr_sdk INTERFACE
)

target_link_libraries(milandr_sdk INTERFACE
)