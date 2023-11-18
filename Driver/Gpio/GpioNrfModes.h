#pragma once

#include <cstdint>
#include "nrf.h"
#include <nrfx_gpiote.h>

namespace ES::Driver::Gpio {
        enum class PullMode : uint32_t {
        None = NRF_GPIO_PIN_NOPULL,
        Up = NRF_GPIO_PIN_PULLUP,
        Down = NRF_GPIO_PIN_PULLDOWN
    };

    enum class DriveMode : uint32_t {
        PushPull = NRF_GPIO_PIN_S0S1,
        OpenDrain = NRF_GPIO_PIN_S0D1,
        HighDrive0Standart1 = NRF_GPIO_PIN_H0S1,
        Standart0HighDrive1 = NRF_GPIO_PIN_S0H1,
        HighDrive0HighDrive1 = NRF_GPIO_PIN_H0H1,
        Disconnect0Standart1 = NRF_GPIO_PIN_D0S1,
        Disconnect0HighDrive1 = NRF_GPIO_PIN_D0H1,
        HighDrive0Disconnect1 = NRF_GPIO_PIN_H0D1
    };

    enum class PinMode : uint32_t {
        Input = NRF_GPIO_PIN_DIR_INPUT,
        Output = NRF_GPIO_PIN_DIR_OUTPUT
    };

    enum class InterruptMode : uint32_t {
        None = 0,
        Rising = NRF_GPIOTE_POLARITY_LOTOHI,
        Falling = NRF_GPIOTE_POLARITY_HITOLO,
        Both = NRF_GPIOTE_POLARITY_TOGGLE
    };

    enum class InitialValue : uint32_t {
		Low = NRF_GPIOTE_INITIAL_VALUE_LOW,
		High = NRF_GPIOTE_INITIAL_VALUE_HIGH
	};
}