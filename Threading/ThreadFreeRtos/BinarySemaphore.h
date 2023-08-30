#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace ES::Threading {
    class BinarySemaphore {
        public:
		BinarySemaphore() {
			_handle = xSemaphoreCreateBinary();
		}

        ~BinarySemaphore() {
			vSemaphoreDelete(_handle);
		}

        bool take(TickType_t wait = portMAX_DELAY) {
            return xSemaphoreTake(_handle, wait) == pdTRUE;
        }

        bool give() {
            return xSemaphoreGive(_handle) == pdTRUE;
        }

        SemaphoreHandle_t getHandle() {
			return _handle;
		}

        private:
        	SemaphoreHandle_t _handle;
    };
}