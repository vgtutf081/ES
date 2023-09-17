#if defined(ES_NRF_GPIOTE)

#include "GpioteNrf52.h"

namespace ES::Driver::Gpiote {

    
    void gpioInEventHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action){

    }

     Nrf52Gpiote::Nrf52Gpiote(uint32_t portPin, PinMode mode, nrf_gpio_pin_pull_t pullmode) : _portPin(portPin) {
            nrfx_err_t err_code;
            if(mode == PinMode::Output) {
                nrfx_gpiote_out_config_t gpioteConfig;
                gpioteConfig.action = NRF_GPIOTE_POLARITY_TOGGLE;
                gpioteConfig.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW;
                gpioteConfig.task_pin = true;

                err_code = nrfx_gpiote_out_init(portPin, &gpioteConfig);
            }

            else if(mode == PinMode::Input) {
                nrfx_gpiote_in_config_t gpioteConfig;
                gpioteConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
                gpioteConfig.pull = pullmode;
                gpioteConfig.is_watcher = false;
                gpioteConfig.hi_accuracy = false;
                gpioteConfig.skip_gpio_setup = false;

                err_code = nrfx_gpiote_in_init(portPin, &gpioteConfig, gpioInEventHandler);
            }

            APP_ERROR_CHECK(err_code);
    }
}

#endif