include(Toolchain/arm-none-eabi)

set(MCU_ARCH cortex-m4)

set(COMMON_COMPILE_FLAGS -mcpu=${MCU_ARCH} -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16)
set(C_CXX_COMPILE_FLAGS  -ffunction-sections -fdata-sections -fsigned-char -g -fmessage-length=0)

add_library(NRF INTERFACE IMPORTED)

target_compile_options(NRF INTERFACE 
    --sysroot="${TOOLCHAIN_SYSROOT}"
    ${COMMON_COMPILE_FLAGS}
)

target_compile_options(NRF INTERFACE $<$<COMPILE_LANGUAGE:C>:${C_CXX_COMPILE_FLAGS}>)
target_compile_options(NRF INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${C_CXX_COMPILE_FLAGS}>)
target_compile_options(NRF INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics>)

target_compile_options(NRF INTERFACE $<$<COMPILE_LANGUAGE:C>:-O0 -g>)
target_compile_options(NRF INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-O0 -g>)
target_compile_options(NRF INTERFACE $<$<COMPILE_LANGUAGE:ASM>:-g>)

target_link_options(NRF INTERFACE 
    --sysroot="${TOOLCHAIN_SYSROOT}" 
    ${COMMON_COMPILE_FLAGS}
    ${C_CXX_COMPILE_FLAGS} 
    LINKER:-gc-sections 
)


#target_link_options(NRF INTERFACE $<$<CONFIG:RELEASE>:LINKER:--undefined=vTaskSwitchContext,--undefined=HardFault_HandlerC,-undefined=_realloc_r>)

target_compile_definitions(NRF INTERFACE
    NRF
    #NRF52840_XXAA
    #BSP_DEFINES_ONLY
    #CONFIG_GPIO_AS_PINRESET
    FLOAT_ABI_HARD
    #FREERTOS
    #USE_APP_CONFIG
    #"__weak=__attribute__((weak))"
)