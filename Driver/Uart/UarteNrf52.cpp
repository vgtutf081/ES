#include "UarteNrf52.h" 

#if defined(ES_NRF_UART)



namespace ES::Driver::Uarte {

    UarteNrf::UarteNrf(uint32_t txPin, uint32_t rxPin, nrf_uarte_baudrate_t baudrate) : _txPin(txPin), _rxPin(rxPin), _baudrate(baudrate)

    {
    instance.drv_inst_idx = NRFX_CONCAT_3(NRFX_UARTE, 0, _INST_IDX);
    instance.p_reg = NRFX_CONCAT_2(NRF_UARTE, 0);

    const nrfx_uarte_config_t uarteConfig =
    {
        txPin,
        rxPin,
        NRF_UARTE_PSEL_DISCONNECTED,
        NRF_UARTE_PSEL_DISCONNECTED,
        nullptr,
        NRF_UARTE_HWFC_DISABLED,
        NRF_UARTE_PARITY_EXCLUDED,
        baudrate,
        APP_IRQ_PRIORITY_HIGHEST
    };

    err_code = nrfx_uarte_init( &instance,
                                &uarteConfig,
                                eventHandler);

    APP_ERROR_CHECK(err_code);
    }

}

#endif