#pragma once

#include "nrf_gpio.h"
#include "nrfx_uarte.h"
#include "nrf_drv_common.h"
#include <cstring>

#include "PinMap.h"

void eventHandler (nrfx_uarte_event_t const * p_event, void * p_context) __attribute__((weak));

namespace ES::Driver::Uarte {

    class UarteNrf {
		public:
		UarteNrf(uint32_t txPin, uint32_t rxPin, nrf_uarte_baudrate_t baudrate);

        ret_code_t writeStream(const char* dataPtr, size_t length) {
            auto sendBuffer = reinterpret_cast<const uint8_t*>(dataPtr);
            memcpy(buf, dataPtr, length);
			sendBuffer = buf;
            err_code = nrfx_uarte_tx(&instance, sendBuffer, length);
            return err_code;
        }

        ret_code_t getStream(uint8_t * dataPtr, size_t length) {
            return nrfx_uarte_rx(&instance, dataPtr, length);
        }
        			
        private:
        uint8_t buf[256];
        const uint8_t* sendBuffer;
        ret_code_t err_code;
        nrfx_uarte_t instance;
        uint32_t _txPin;
        uint32_t _rxPin;
        nrf_uarte_baudrate_t _baudrate;
    };

}