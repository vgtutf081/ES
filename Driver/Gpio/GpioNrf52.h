#pragma once

#include "nrf_gpio.h"

namespace ES::Driver::Gpio {

    enum class PinMode : uint32_t {
        Input = NRF_GPIO_PIN_DIR_INPUT,
        Output = NRF_GPIO_PIN_DIR_OUTPUT
    };

    enum class PullMode : uint32_t {
        NoPull = NRF_GPIO_PIN_DIR_INPUT,
        PullDown = NRF_GPIO_PIN_DIR_OUTPUT,
        PullUp = NRF_GPIO_PIN_PULLUP
    };

    enum class DriveMode : uint32_t {
        PushPull = NRF_GPIO_PIN_S0S1,
        OpenDrain = NRF_GPIO_PIN_S0D1
    };

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