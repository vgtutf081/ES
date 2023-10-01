#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace ES::Threading {
    class Semaphore {
    public:

    static constexpr TickType_t maxDelay = portMAX_DELAY;
        
        bool take(TickType_t wait = portMAX_DELAY) {
			return xSemaphoreTake(_handle, wait) == pdTRUE;
		}

        bool takeFromIsr() {
            return (xSemaphoreTakeFromISR(_handle, pdFALSE) != pdTRUE);
        }

        ~Semaphore() {
            vSemaphoreDelete(_handle);
        }

        bool give(bool yieldFromISR = true) {
			return xSemaphoreGive(_handle) == pdTRUE;
        }

        bool giveFromIsr(bool yieldFromISR = true) {
			return (xSemaphoreGiveFromISR(_handle, pdFALSE) != pdTRUE);

        }

        SemaphoreHandle_t getHandle() {
			return _handle;
		}

    protected:
        SemaphoreHandle_t _handle;
    };


    class BinarySemaphore : public Semaphore {
	public:
		BinarySemaphore() {
			_handle = xSemaphoreCreateBinary();
		}
	};
}