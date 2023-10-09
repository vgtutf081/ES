#pragma once

#include "II2C.h"
#include "nrfx_twim.h"

namespace ES::Driver::I2C {
    class I2CNrf52 : public II2C{
    public:
        explicit I2CNrf52 (nrfx_twim_t instance, uint32_t sda, uint32_t scl, nrf_twim_frequency_t frequency = NRF_TWIM_FREQ_400K) : _instance(instance), _sda(sda), _scl(scl), _frequency(frequency) {

        }

        void init(void* context) {
            nrfx_twim_config_t twi_config;
            twi_config.scl                = _scl;
            twi_config.sda                = _sda;

            twi_config.frequency          = _frequency;
            twi_config.interrupt_priority = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY;
            twi_config.hold_bus_uninit    = false;

            nrfx_twim_init(&_instance, &twi_config, i2cEventHandlerStatic, context);
            nrfx_twim_enable(&_instance);
        }



        ~I2CNrf52() {
            nrfx_twim_uninit(&_instance);
        }

        static void i2cEventHandlerStatic(nrfx_twim_evt_t const * p_event, void * p_context){
            static_cast<void>(p_event);
            I2CNrf52* this = reinterpret_cast<I2CNrf52*>(p_context);
            this->i2cEventHandler(p_event->type == nrfx_twim_evt_type_t::NRFX_TWIM_EVT_DONE);
        }

    private:
        nrf_twim_frequency_t _frequency;
        nrfx_twim_t _instance;
        uint32_t _sda;
        uint32_t _scl;
    };
}