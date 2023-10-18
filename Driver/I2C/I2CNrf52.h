#pragma once

#include "II2C.h"
#include "nrfx_twim.h"

#include "ThreadFreeRtos.h"

#include <array>

namespace ES::Driver::I2C {

    struct I2Сcallback {
        typedef void(*EventCallback)(nrfx_twim_evt_type_t);
        EventCallback callback = nullptr;
    };

    class I2CNrf52 : public II2C{
    public:
        explicit I2CNrf52 (nrfx_twim_t instance, uint32_t sda, uint32_t scl, I2Сcallback* callback, nrf_twim_frequency_t frequency = NRF_TWIM_FREQ_400K) : _instance(instance), _sda(sda), _scl(scl), _callback(callback), _frequency(frequency) {
            ret_code_t ret;
            nrfx_twim_config_t twi_config;
            twi_config.scl                = _scl;
            twi_config.sda                = _sda;

            twi_config.frequency          = _frequency;
            twi_config.interrupt_priority = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY;
            twi_config.hold_bus_uninit    = false;

            ret = nrfx_twim_init(&_instance, &twi_config, i2cEventHandlerStatic, this);
            if(ret == NRF_SUCCESS) {
                nrfx_twim_enable(&_instance);
            }
        }

        bool read(uint16_t address, uint8_t *buffer, size_t size) override {
            ret_code_t ret;
            yieldForBusy();
            ret = nrfx_twim_rx(&_instance, address >> 1, buffer, size);
            return ret == NRF_SUCCESS;
        }

        bool write(uint16_t address, const uint8_t *buffer, size_t size) override {
            ret_code_t ret;
            
            auto pointer = buffer;

        	if(!nrfx_is_in_ram(pointer)) {
        		std::memcpy(_buf.data(), pointer, size);
        		pointer = _buf.data();
        	}
            yieldForBusy();
            ret = nrfx_twim_tx(&_instance, address >> 1, static_cast<const uint8_t*>(pointer), size, false);

            return ret == NRF_SUCCESS;
        }

        bool read(uint16_t address, std::uint16_t memAddress, size_t memAddressBitCount, uint8_t* buffer, size_t size) override {
            bool result = false;
            size_t transferSize = pasteAddress(memAddress, memAddressBitCount);
            
            result = write(address, &_buf[0], transferSize);
            if (result) {
                result = read(address, buffer, size);
            }
            return result;
        }
        
        bool write(uint16_t address, std::uint16_t memAddress, size_t memAddressBitCount, const uint8_t* buffer, size_t size) override {
            bool result = false;

            size_t transferSize = (memAddressBitCount > 8 ? 2 : 1) + size;
            if (transferSize > _buf.size()) {
                return result;
            }

            std::size_t beginData = pasteAddress(memAddress, memAddressBitCount);

            std::memcpy(&_buf[beginData], buffer, size);

            result = write(address, _buf.data(), transferSize);

            return result;
        }

        ~I2CNrf52() {
            nrfx_twim_uninit(&_instance);
        }

    private:

        inline void yieldForBusy() {
            while(nrfx_twim_is_busy(&_instance)) {
                Threading::yield();
            }
        }

        static void i2cEventHandlerStatic(nrfx_twim_evt_t const * p_event, void * p_context) {
            //static_cast<void>(p_event);
            I2CNrf52* _this = reinterpret_cast<I2CNrf52*>(p_context);
            _this->i2cEventHandler(p_event->type);
        }

        void i2cEventHandler(nrfx_twim_evt_type_t p_event) {
            if(_callback->callback != nullptr) {
                _callback->callback(p_event);
            }
        }

        size_t pasteAddress(const std::uint16_t& memAddress, const std::uint16_t& memAddressBitCount) {
            if (memAddressBitCount > 8) {
                _buf[0] = memAddress >> 8;
                _buf[1] = memAddress & 0xFF;
                return 2;
            } else {
                _buf[0] = static_cast<uint8_t>(memAddress);
                return 1;
            }
        }

        I2Сcallback* _callback;

        nrf_twim_frequency_t _frequency;
        nrfx_twim_t _instance;
        uint32_t _sda;
        uint32_t _scl;
        std::array<uint8_t, 256> _buf;
    };
}