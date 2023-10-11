include(NRF/NrfCommon)

#set(MCU_FLOAT_ABI softfp)
#set(MCU_FPU fpv4-sp-d16)

target_compile_definitions(NRF INTERFACE
    NRF52840_XXAA
)