#pragma once

#include "nrf_gpio.h"
#include "GpioNrfModes.h"
#include "IGpio.h"
#include <limits>
//#include "ExtiCommon.h"

#include "CommonTools.h"

#define NRF_PINS_IN_PORT	32
#define NRF_GPIO_MASK	(NRF_PINS_IN_PORT - 1)

namespace ES::Driver::Gpio {

    class Nrf52Gpio : public IGpio {
    public:
        enum class Port : uint32_t{
            Port0 = 0
#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
            ,Port1 = 1
#endif
            };

    static constexpr uint32_t getPortAddress(uint32_t pinNumber) {
		#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
			if(pinNumber >= NRF_PINS_IN_PORT) {
				return ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS(NRF_P1);
			}
			else {
				return ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS(NRF_P0);
			}
		#else
			# error "Unknown chip"
		#endif
	    }

        static constexpr uint32_t getPortId(uint32_t portAddress) {
            if(portAddress == ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS(NRF_P0)) {
                return 0;
            }
#if defined(NRF52840_XXAA) || defined(NRF52833_XXAA)
            if(portAddress == ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS(NRF_P1)) {
                return 1;
            }
#endif
		    return (std::numeric_limits<uint32_t>::max)();
	    }

        static constexpr size_t PortInIdOffset = 5;

        constexpr Nrf52Gpio(uint32_t nrfPortAndPin) : portPin(nrfPortAndPin), portAddress(getPortAddress(nrfPortAndPin)), pinMask(1U << (nrfPortAndPin & NRF_GPIO_MASK)), pin(nrfPortAndPin & NRF_GPIO_MASK) {

        }

        constexpr Nrf52Gpio();

        static void gpioteHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action, void* context) {
            static_cast<void>(action);
            //reinterpret_cast<InterruptCallback*>(context)->onGpioInterrupt(ID{pin});
        }

        /*void configureInterrupt(PullMode pullMode, InterruptMode interruptMode, InterruptCallback* callback) {
            nrfx_gpiote_in_config_t gpioteConfig;
            gpioteConfig.is_watcher = false;
            gpioteConfig.hi_accuracy = true;
            gpioteConfig.pull = static_cast<nrf_gpio_pin_pull_t>(pullMode);
            gpioteConfig.sense = static_cast<nrf_gpiote_polarity_t>(interruptMode);
            gpioteConfig.skip_gpio_setup = false;
            if(nrfx_gpiote_in_init(getPortAndPin(), &gpioteConfig, gpioteHandler, callback) != NRFX_SUCCESS) {
                return;
            }
            nrfx_gpiote_in_event_enable(getPortAndPin(), true);
        }*/

        void set() override {
            getPortPtr()->OUTSET = pinMask;
        }

        void reset() override {
            getPortPtr()->OUTCLR = pinMask;
        }

        void toggle() override {
            if((getPortPtr()->OUT & pinMask) != 0){
                reset();
            }
            else {
                set();
            }
        }

        bool read() const override { 
            return (getPortPtr()->IN & pinMask) != 0;
        }

        void disable() override {
            getPortPtr()->PIN_CNF[pin] = GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos;
        }

        void setPullMode(PullMode mode) override {
            auto port = getPortPtr();
            auto cnf = (port->PIN_CNF[pin] & ~(GPIO_PIN_CNF_PULL_Msk));
            port->PIN_CNF[pin] = cnf | static_cast<uint32_t>(mode) << GPIO_PIN_CNF_PULL_Pos;
        }

        void setMode(PinMode mode) override {
            auto port = getPortPtr();
            auto cnf = (port->PIN_CNF[pin] & ~(GPIO_PIN_CNF_DIR_Msk));
            if(mode == PinMode::Input) {
                cnf &= ~GPIO_PIN_CNF_INPUT_Msk;
                cnf |= GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos;
            }
            port->PIN_CNF[pin] = cnf | static_cast<uint32_t>(mode) << GPIO_PIN_CNF_DIR_Pos;
        }


        void setDriveMode(DriveMode mode) override {
            auto port = getPortPtr();
            auto cnf = port->PIN_CNF[pin] & ~(GPIO_PIN_CNF_DRIVE_Msk);
            if(mode != DriveMode::PushPull && mode != DriveMode::HighDrive0HighDrive1) {
                cnf &= ~GPIO_PIN_CNF_INPUT_Msk;
                cnf |= GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos;
            }
            port->PIN_CNF[pin] = cnf | static_cast<uint32_t>(mode) << GPIO_PIN_CNF_DRIVE_Pos;
        }

        NRF_GPIO_Type* getPortPtr() const {
            return reinterpret_cast<NRF_GPIO_Type*>(portAddress);
        }

        uint32_t getPort() const override {
            return portAddress;
        }

        uint32_t getPin() const override {
            return portPin;
        }

        uint32_t portPin;
        uint32_t portAddress;
        uint32_t pinMask;
        uint32_t pin;

        uint32_t getPortAndPin() const override {
            return (getPortId(portAddress) << PortInIdOffset) | pin;
        }

    private:
    };
}