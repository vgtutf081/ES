set(PROJECT_NAME NrfSdk)
add_library(${PROJECT_NAME} INTERFACE )

target_link_options(${PROJECT_NAME} INTERFACE 
    -L ${CMAKE_CURRENT_LIST_DIR}/modules/nrfx/mdk
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/components/drivers_nrf/nrf_soc_nosd"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/boards"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/toolchain"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/toolchain/cmsis/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/util"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/fifo"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/usbd"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/log"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/log/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/balloc"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/atomic"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/delay"
        "${CMAKE_CURRENT_SOURCE_DIR}/components/libraries/experimental_section_vars"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/nrfx"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/nrfx/mdk"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/nrfx/hal"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/nrfx/drivers/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/integration/nrfx"
        "${CMAKE_CURRENT_SOURCE_DIR}/integration/nrfx/legacy"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/utf_converter"
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
)

target_sources(${PROJECT_NAME} 
INTERFACE
    #modules/nrfx/mdk/gcc_startup_nrf52.S
    #modules/nrfx/mdk/gcc_startup_nrf52805.S
    #modules/nrfx/mdk/gcc_startup_nrf52810.S
    #modules/nrfx/mdk/gcc_startup_nrf52811.S
    #modules/nrfx/mdk/gcc_startup_nrf52820.S
    #modules/nrfx/mdk/gcc_startup_nrf52833.S
    modules/nrfx/mdk/gcc_startup_nrf52840.S
)

target_sources(${PROJECT_NAME} 
INTERFACE
    modules/nrfx/mdk/system_nrf.h
    modules/nrfx/mdk/system_nrf51.c
    modules/nrfx/mdk/system_nrf51.h
    modules/nrfx/mdk/system_nrf52.c
    modules/nrfx/mdk/system_nrf52.h
    modules/nrfx/mdk/system_nrf52805.c
    modules/nrfx/mdk/system_nrf52805.h
    modules/nrfx/mdk/system_nrf52810.c
    modules/nrfx/mdk/system_nrf52810.h
    modules/nrfx/mdk/system_nrf52811.c
    modules/nrfx/mdk/system_nrf52811.h
    modules/nrfx/mdk/system_nrf52820.c
    modules/nrfx/mdk/system_nrf52820.h
    modules/nrfx/mdk/system_nrf52833.c
    modules/nrfx/mdk/system_nrf52833.h
    modules/nrfx/mdk/system_nrf52840.c
    modules/nrfx/mdk/system_nrf52840.h
)



target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/atomic/nrf_atomic.c
    components/libraries/atomic/nrf_atomic.h
    components/libraries/atomic/nrf_atomic_internal.h
    components/libraries/atomic/nrf_atomic_sanity_check.h
)

target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/atomic_fifo/nrf_atfifo.c
    components/libraries/atomic_fifo/nrf_atfifo.h
    components/libraries/atomic_fifo/nrf_atfifo_internal.h
)

target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/atomic_flags/nrf_atflags.c
    components/libraries/atomic_flags/nrf_atflags.h
)

target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/balloc/nrf_balloc.c
    components/libraries/balloc/nrf_balloc.h
)

target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/delay/nrf_delay.h
)

target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/experimental_section_vars/nrf_section.h
    components/libraries/experimental_section_vars/nrf_section_iter.c
    components/libraries/experimental_section_vars/nrf_section_iter.h
)


target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/fifo/app_fifo.c
    components/libraries/fifo/app_fifo.h
)



#components/drivers_nrf/nrf_soc_nosd
target_sources(${PROJECT_NAME} 
INTERFACE
    components/drivers_nrf/nrf_soc_nosd/nrf_error.h
    components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c
    components/drivers_nrf/nrf_soc_nosd/nrf_nvic.h
    components/drivers_nrf/nrf_soc_nosd/nrf_sdm.h
    components/drivers_nrf/nrf_soc_nosd/nrf_soc.c
    components/drivers_nrf/nrf_soc_nosd/nrf_soc.h
)

target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/log/nrf_log.h
    components/libraries/log/nrf_log_backend_flash.h
    components/libraries/log/nrf_log_backend_interface.h
    components/libraries/log/nrf_log_backend_rtt.h
    components/libraries/log/nrf_log_backend_uart.h
    components/libraries/log/nrf_log_ctrl.h
    components/libraries/log/nrf_log_default_backends.h
    components/libraries/log/nrf_log_instance.h
    components/libraries/log/nrf_log_str_formatter.h
    components/libraries/log/nrf_log_types.h
    components/libraries/log/src/nrf_log_backend_flash.c
    components/libraries/log/src/nrf_log_backend_rtt.c
    components/libraries/log/src/nrf_log_backend_serial.c
    components/libraries/log/src/nrf_log_backend_serial.h
    components/libraries/log/src/nrf_log_backend_uart.c
    components/libraries/log/src/nrf_log_ctrl_internal.h
    components/libraries/log/src/nrf_log_default_backends.c
    components/libraries/log/src/nrf_log_frontend.c
    components/libraries/log/src/nrf_log_internal.h
    components/libraries/log/src/nrf_log_str_formatter.c
)

if(${USE_USB_LIBRARY})
target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/usbd/app_usbd.c
    components/libraries/usbd/app_usbd.h
    components/libraries/usbd/app_usbd_class_base.h
    components/libraries/usbd/app_usbd_core.c
    components/libraries/usbd/app_usbd_core.h
    components/libraries/usbd/app_usbd_descriptor.h
    components/libraries/usbd/app_usbd_langid.h
    components/libraries/usbd/app_usbd_request.h
    components/libraries/usbd/app_usbd_serial_num.c
    components/libraries/usbd/app_usbd_serial_num.h
    components/libraries/usbd/app_usbd_string_desc.c
    components/libraries/usbd/app_usbd_string_desc.h
    components/libraries/usbd/app_usbd_types.h
)
endif()


target_sources(${PROJECT_NAME} 
INTERFACE
    components/libraries/util/app_error.c
    components/libraries/util/app_error.h
    components/libraries/util/app_error_handler_gcc.c
    #components/libraries/util/app_error_weak.c
    #components/libraries/util/app_error_weak.h
    components/libraries/util/app_util.h
    components/libraries/util/app_util_bds.h
    components/libraries/util/app_util_platform.c
    components/libraries/util/app_util_platform.h
    components/libraries/util/nordic_common.h
    components/libraries/util/nrf_assert.c
    components/libraries/util/nrf_assert.h
    components/libraries/util/nrf_bitmask.h
    components/libraries/util/sdk_alloca.h
    components/libraries/util/sdk_common.h
    components/libraries/util/sdk_errors.h
    components/libraries/util/sdk_macros.h
    components/libraries/util/sdk_mapped_flags.c
    components/libraries/util/sdk_mapped_flags.h
    components/libraries/util/sdk_os.h
    components/libraries/util/sdk_resources.h
)



target_sources(${PROJECT_NAME} 
INTERFACE
    modules/nrfx/soc/nrfx_atomic.c
    modules/nrfx/soc/nrfx_atomic.h
    modules/nrfx/soc/nrfx_atomic_internal.h
    modules/nrfx/soc/nrfx_coredep.h
    modules/nrfx/soc/nrfx_irqs.h
    modules/nrfx/soc/nrfx_irqs_nrf51.h
    modules/nrfx/soc/nrfx_irqs_nrf52810.h
    modules/nrfx/soc/nrfx_irqs_nrf52811.h
    modules/nrfx/soc/nrfx_irqs_nrf52820.h
    modules/nrfx/soc/nrfx_irqs_nrf52832.h
    modules/nrfx/soc/nrfx_irqs_nrf52833.h
    modules/nrfx/soc/nrfx_irqs_nrf52840.h
    modules/nrfx/soc/nrfx_irqs_nrf9160.h
)



target_sources(${PROJECT_NAME} 
INTERFACE
    integration/nrfx/legacy/apply_old_config.h
    integration/nrfx/legacy/nrf_drv_clock.c
    integration/nrfx/legacy/nrf_drv_clock.h
    integration/nrfx/legacy/nrf_drv_common.h
    integration/nrfx/legacy/nrf_drv_comp.h
    integration/nrfx/legacy/nrf_drv_gpiote.h
    integration/nrfx/legacy/nrf_drv_i2s.h
    integration/nrfx/legacy/nrf_drv_lpcomp.h
    integration/nrfx/legacy/nrf_drv_pdm.h
    integration/nrfx/legacy/nrf_drv_power.c
    integration/nrfx/legacy/nrf_drv_power.h
    integration/nrfx/legacy/nrf_drv_ppi.c
    integration/nrfx/legacy/nrf_drv_ppi.h
    integration/nrfx/legacy/nrf_drv_pwm.h
    integration/nrfx/legacy/nrf_drv_qdec.h
    integration/nrfx/legacy/nrf_drv_qspi.h
    integration/nrfx/legacy/nrf_drv_rng.c
    integration/nrfx/legacy/nrf_drv_rng.h
    integration/nrfx/legacy/nrf_drv_rtc.h
    integration/nrfx/legacy/nrf_drv_saadc.h
    integration/nrfx/legacy/nrf_drv_spi.c
    integration/nrfx/legacy/nrf_drv_spi.h
    integration/nrfx/legacy/nrf_drv_spis.c
    integration/nrfx/legacy/nrf_drv_spis.h
    integration/nrfx/legacy/nrf_drv_swi.c
    integration/nrfx/legacy/nrf_drv_swi.h
    integration/nrfx/legacy/nrf_drv_systick.h
    integration/nrfx/legacy/nrf_drv_timer.h
    integration/nrfx/legacy/nrf_drv_twi.c
    integration/nrfx/legacy/nrf_drv_twi.h
    integration/nrfx/legacy/nrf_drv_twis.h
    integration/nrfx/legacy/nrf_drv_uart.c
    integration/nrfx/legacy/nrf_drv_uart.h
    integration/nrfx/legacy/nrf_drv_usbd.h
    integration/nrfx/legacy/nrf_drv_usbd_errata.h
    integration/nrfx/legacy/nrf_drv_wdt.h
)





target_sources(${PROJECT_NAME} 
INTERFACE
    modules/nrfx/drivers/nrfx_common.h
    modules/nrfx/drivers/nrfx_errors.h
    modules/nrfx/drivers/include/nrfx_adc.h
    modules/nrfx/drivers/include/nrfx_clock.h
    modules/nrfx/drivers/include/nrfx_comp.h
    modules/nrfx/drivers/include/nrfx_dppi.h
    modules/nrfx/drivers/include/nrfx_gpiote.h
    modules/nrfx/drivers/include/nrfx_i2s.h
    modules/nrfx/drivers/include/nrfx_lpcomp.h
    modules/nrfx/drivers/include/nrfx_nfct.h
    modules/nrfx/drivers/include/nrfx_nvmc.h
    modules/nrfx/drivers/include/nrfx_pdm.h
    modules/nrfx/drivers/include/nrfx_power.h
    modules/nrfx/drivers/include/nrfx_power_clock.h
    modules/nrfx/drivers/include/nrfx_ppi.h
    modules/nrfx/drivers/include/nrfx_pwm.h
    modules/nrfx/drivers/include/nrfx_qdec.h
    modules/nrfx/drivers/include/nrfx_qspi.h
    modules/nrfx/drivers/include/nrfx_rng.h
    modules/nrfx/drivers/include/nrfx_rtc.h
    modules/nrfx/drivers/include/nrfx_saadc.h
    modules/nrfx/drivers/include/nrfx_saadc_v2.h
    modules/nrfx/drivers/include/nrfx_spi.h
    modules/nrfx/drivers/include/nrfx_spim.h
    modules/nrfx/drivers/include/nrfx_spis.h
    modules/nrfx/drivers/include/nrfx_swi.h
    modules/nrfx/drivers/include/nrfx_systick.h
    modules/nrfx/drivers/include/nrfx_temp.h
    modules/nrfx/drivers/include/nrfx_timer.h
    modules/nrfx/drivers/include/nrfx_twi.h
    modules/nrfx/drivers/include/nrfx_twim.h
    modules/nrfx/drivers/include/nrfx_twis.h
    modules/nrfx/drivers/include/nrfx_twi_twim.h
    modules/nrfx/drivers/include/nrfx_uart.h
    modules/nrfx/drivers/include/nrfx_uarte.h
    modules/nrfx/drivers/include/nrfx_usbd.h
    modules/nrfx/drivers/include/nrfx_wdt.h
    modules/nrfx/drivers/include/nrf_bitmask.h
)



target_sources(${PROJECT_NAME} 
INTERFACE
    modules/nrfx/drivers/src/nrfx_adc.c
    modules/nrfx/drivers/src/nrfx_clock.c
    modules/nrfx/drivers/src/nrfx_comp.c
    modules/nrfx/drivers/src/nrfx_dppi.c
    modules/nrfx/drivers/src/nrfx_gpiote.c
    modules/nrfx/drivers/src/nrfx_i2s.c
    modules/nrfx/drivers/src/nrfx_lpcomp.c
    modules/nrfx/drivers/src/nrfx_nfct.c
    modules/nrfx/drivers/src/nrfx_nvmc.c
    modules/nrfx/drivers/src/nrfx_pdm.c
    modules/nrfx/drivers/src/nrfx_power.c
    modules/nrfx/drivers/src/nrfx_ppi.c
    modules/nrfx/drivers/src/nrfx_pwm.c
    modules/nrfx/drivers/src/nrfx_qdec.c
    modules/nrfx/drivers/src/nrfx_qspi.c
    modules/nrfx/drivers/src/nrfx_rng.c
    modules/nrfx/drivers/src/nrfx_rtc.c
    modules/nrfx/drivers/src/nrfx_saadc.c
    modules/nrfx/drivers/src/nrfx_spi.c
    modules/nrfx/drivers/src/nrfx_spim.c
    modules/nrfx/drivers/src/nrfx_spis.c
    modules/nrfx/drivers/src/nrfx_swi.c
    modules/nrfx/drivers/src/nrfx_systick.c
    modules/nrfx/drivers/src/nrfx_temp.c
    modules/nrfx/drivers/src/nrfx_timer.c
    modules/nrfx/drivers/src/nrfx_twi.c
    modules/nrfx/drivers/src/nrfx_twim.c
    modules/nrfx/drivers/src/nrfx_twis.c
    modules/nrfx/drivers/src/nrfx_twi_twim.c
    modules/nrfx/drivers/src/nrfx_uart.c
    modules/nrfx/drivers/src/nrfx_uarte.c
    modules/nrfx/drivers/src/nrfx_usbd.c
    modules/nrfx/drivers/src/nrfx_usbd_errata.h
    modules/nrfx/drivers/src/nrfx_wdt.c
    modules/nrfx/drivers/src/prs/nrfx_prs.c
    modules/nrfx/drivers/src/prs/nrfx_prs.h
)
