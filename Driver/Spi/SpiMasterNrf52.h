#pragma once

#include "SpiMaster.h"
#include "nrfx_spim.h"
#include "GpioNrf52.h"

namespace ES::Driver {

    class SpiMaster : public ISpiMaster {
    enum class State {
        Idle,
        Transfer
    };
    public:

    SpiMaster(nrfx_spim_t spi, uint32_t cs, uint32_t miso, uint32_t mosi, uint32_t sck,
            nrf_spim_bit_order_t bit_order = NRF_SPIM_BIT_ORDER_MSB_FIRST,
            nrf_spim_frequency_t frequency = NRF_SPIM_FREQ_8M,
            nrf_spim_mode_t mode = NRF_SPIM_MODE_0) :
        _spi(spi), _cs({cs}) {

        _cs.set();
        _cs.configureOutput();

        nrfx_spim_config_t	config;
        config.bit_order = bit_order;
        config.frequency = frequency;
        config.irq_priority = NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY;
        config.sck_pin = sck;
        config.miso_pin = miso;
        config.mosi_pin = mosi;
        config.ss_pin = NRFX_SPIM_PIN_NOT_USED;
        config.orc = 0x00;
        config.ss_active_high = false;
        config.mode = mode;

        auto status = nrfx_spim_init(&_spi,
                        &config,
                        spiEventHandler,
                        this);
        asm("nop");
        NRFX_ASSERT(status == NRF_SUCCESS);
    };

    ~SpiMaster() {
        _cs.disable();
        nrfx_spim_uninit(&_spi);
    }

    void setEventHandler(ISpiEventHandler* listener) final {
        _listener = listener;
    }

    static void spiEventHandler(nrfx_spim_evt_t const * p_event, void * p_context){
        static_cast<void>(p_event);
        SpiMaster* _this = reinterpret_cast<SpiMaster*>(p_context);
        _this->_cs.set();
        _this->_state = State::Idle;
        if (_this->_listener != nullptr) {
            _this->_listener->onTransferComplete();
        }
    }

    void read(uint8_t* rxBuffer, size_t size) override {
        readWrite(0, rxBuffer, size);
    }

    void write(const uint8_t* txBuffer, size_t size) override {
        readWrite(txBuffer, 0, size);
    }


    bool readWrite(const uint8_t* txBuffer, uint8_t* rxBuffer, size_t size) {
        if (_state == State::Transfer) return false;
        _xferDesc.p_tx_buffer = txBuffer;
        _xferDesc.p_rx_buffer = rxBuffer;
        _xferDesc.tx_length = size;
        _xferDesc.rx_length = size;
        _cs.reset();
        _state = State::Transfer;
        bool success =  (nrfx_spim_xfer(&_spi, &_xferDesc, 0) == NRFX_SUCCESS);
        if (!success){
            _cs.set();
            _state = State::Idle;
        }
        return success;
    }

    bool readWrite(const uint8_t* txBuffer, uint8_t* rxBuffer, size_t sizeTx, size_t sizeRx) {
        if (_state == State::Transfer) return false;
        _xferDesc.p_tx_buffer = txBuffer;
        _xferDesc.p_rx_buffer = rxBuffer;
        _xferDesc.tx_length = sizeTx;
        _xferDesc.rx_length = sizeRx;
        _cs.reset();
        _state = State::Transfer;
        bool success =  (nrfx_spim_xfer(&_spi, &_xferDesc, 0) == NRFX_SUCCESS);
        if (!success){
            _cs.set();
            _state = State::Idle;
        }
        return success;
    }

    State getState() {
        return _state;
    }

    private:
    State									_state = State::Idle;
    nrfx_spim_t 							_spi;
    Gpio::Nrf52Gpio _cs;

    nrfx_spim_xfer_desc_t					_xferDesc;
    ISpiEventHandler*					_listener = nullptr;
    };

}
