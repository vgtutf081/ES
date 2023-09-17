#pragma once

#include "nrf_gpio.h"
#include "GpioNrf52.h"
#include "GpioNrfModes.h"

namespace ES::Driver::Gpio {

    struct Nrf52Gpio {
    public:
        Nrf52Gpio(uint32_t portPin, PinMode dir, PullMode pull, DriveMode drive) : _portPin(portPin) {

            nrf_gpio_cfg(_portPin, static_cast<nrf_gpio_pin_dir_t>(dir), NRF_GPIO_PIN_INPUT_DISCONNECT, static_cast<nrf_gpio_pin_pull_t>(pull), static_cast<nrf_gpio_pin_drive_t>(drive), NRF_GPIO_PIN_NOSENSE);
        }

        void set() {
            nrf_gpio_pin_set(_portPin);
        }

        void reset() {
            nrf_gpio_pin_clear(_portPin);
        }

        void toggle() {
            nrf_gpio_pin_toggle(_portPin);
        }

    private:
        uint32_t _portPin;
    };
}