#pragma once

#include "nrf_gpio.h"
#include "nrfx_uarte.h"
#include "nrf_drv_common.h"
#include <cstring>

#include "ThreadFreeRtos.h"

#include "PinMap.h"

namespace ES::Driver::Uarte {

    class UarteNrf {
		public:
		UarteNrf(nrfx_uarte_t instance, uint32_t txPin, uint32_t rxPin, nrf_uarte_baudrate_t baudrate)  : _instance(instance), _txPin(txPin), _rxPin(rxPin), _baudrate(baudrate) {

        }

        ret_code_t init (nrfx_uarte_event_handler_t eventHandler, void * context) {

            nrfx_uarte_config_t uarteConfig =
            {   
                _txPin,
                _rxPin,
                NRF_UARTE_PSEL_DISCONNECTED,
                NRF_UARTE_PSEL_DISCONNECTED,
                context,
                NRF_UARTE_HWFC_DISABLED,
                NRF_UARTE_PARITY_EXCLUDED,
                _baudrate,
                APP_IRQ_PRIORITY_LOWEST
            };
            err_code = nrfx_uarte_init( &_instance,
                                        &uarteConfig,
                                        eventHandler);

            return err_code;
        }

        ret_code_t writeStream(const char* dataPtr, size_t length) {
            auto sendBuffer = reinterpret_cast<const uint8_t*>(dataPtr);
            memcpy(buf, dataPtr, length);
			sendBuffer = buf;
            while(nrfx_uarte_tx_in_progress(&_instance)) {
                Threading::yield();
            }
            /*if(nrfx_uarte_tx_in_progress(&_instance)) {
                stopStream();
            }*/
            err_code = nrfx_uarte_tx(&_instance, sendBuffer, length);
            return err_code;
        }

        ret_code_t getStream(uint8_t * dataPtr, size_t length) {
            return nrfx_uarte_rx(&_instance, dataPtr, length);
        }

        void stopStream() {
            nrfx_uarte_rx_abort(&_instance);
        }
        			
        private:
        uint8_t buf[256];
        const uint8_t* sendBuffer;
        ret_code_t err_code;
        nrf_uarte_baudrate_t _baudrate;
        uint32_t _rxPin;
        uint32_t _txPin;

       nrfx_uarte_t _instance;
    };

}