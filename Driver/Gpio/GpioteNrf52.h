#pragma once

#include "nrfx_gpiote.h"

namespace ES::Driver::Gpiote {

    enum PinMode {
        Input,
        Output
    };

    struct Nrf52Gpiote {
        public:
        Nrf52Gpiote(uint32_t portPin, PinMode mode, nrf_gpio_pin_pull_t pullmode = NRF_GPIO_PIN_NOPULL);

        void set() {
            nrf_gpio_pin_set(_portPin);
        }

        void reset() {
            nrf_gpio_pin_clear(_portPin);
        }

        private:
        uint32_t _portPin;
    };
}