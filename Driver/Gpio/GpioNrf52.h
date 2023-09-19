#pragma once

#include "nrf_gpio.h"
#include "GpioNrfModes.h"
#include "IGpio.h"
#include <nrfx_gpiote.h>

namespace ES::Driver::Gpio {

    struct Nrf52Gpio : public IGpio {
    public:
        Nrf52Gpio(uint32_t portPin) : _portPin(portPin) {

            //nrf_gpio_cfg(_portPin, static_cast<nrf_gpio_pin_dir_t>(dir), NRF_GPIO_PIN_INPUT_DISCONNECT, static_cast<nrf_gpio_pin_pull_t>(pull), static_cast<nrf_gpio_pin_drive_t>(drive), NRF_GPIO_PIN_NOSENSE);
        }

        void setMode(PinMode mode, DriveMode drive, PullMode pull) {
            NRF_GPIO_Type * reg = nrf_gpio_pin_port_decode(&_portPin);

            reg->PIN_CNF[_portPin] = ((uint32_t)static_cast<nrf_gpio_pin_dir_t>(mode) << GPIO_PIN_CNF_DIR_Pos)
                                    | ((uint32_t)static_cast<nrf_gpio_pin_pull_t>(pull) << GPIO_PIN_CNF_PULL_Pos)
                                    | ((uint32_t)static_cast<nrf_gpio_pin_drive_t>(drive) << GPIO_PIN_CNF_DRIVE_Pos)
                                    | ((uint32_t)NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos);

            if(mode == PinMode::Input) {
                reg->PIN_CNF[_portPin] |= ((uint32_t)NRF_GPIO_PIN_INPUT_CONNECT << GPIO_PIN_CNF_INPUT_Pos);
            } else {
                reg->PIN_CNF[_portPin] |= ((uint32_t)NRF_GPIO_PIN_INPUT_DISCONNECT << GPIO_PIN_CNF_INPUT_Pos);
            }
        }

        void configureInterrupt(PullMode pullMode, InterruptMode interruptMode, nrfx_gpiote_evt_handler_t callback, void* context) {
            nrfx_gpiote_in_config_t gpioteConfig;
            gpioteConfig.is_watcher = false;
            gpioteConfig.hi_accuracy = true;
            gpioteConfig.pull = static_cast<nrf_gpio_pin_pull_t>(pullMode);
            gpioteConfig.sense = static_cast<nrf_gpiote_polarity_t>(interruptMode);
            gpioteConfig.skip_gpio_setup = false;
            nrfx_gpiote_in_init(_portPin, &gpioteConfig, callback, context);
            nrfx_gpiote_in_event_enable(_portPin, true);
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

        bool read() {
            return nrf_gpio_pin_read(_portPin);
        }

        void disable() {
            nrf_gpio_input_disconnect(_portPin);
        }

    private:
        uint32_t _portPin;
    };
}