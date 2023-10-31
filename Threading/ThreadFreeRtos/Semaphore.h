#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace ES::Threading {

    bool isInterruptHandling();

    class Semaphore {
    public:

    static constexpr TickType_t maxDelay = portMAX_DELAY;
        
        bool take(TickType_t wait = portMAX_DELAY) {
#if defined(NRF)
            if(isInterruptHandling()) {
                portBASE_TYPE taskWoken = pdFALSE;
                if(!takeFromIsr()) {
                    return false;
                }
                portEND_SWITCHING_ISR(taskWoken);
                return true;
            }
#endif
			return xSemaphoreTake(_handle, wait) == pdTRUE;
		}

        bool takeFromIsr() {
            return (xSemaphoreTakeFromISR(_handle, pdFALSE) != pdTRUE);
        }

        ~Semaphore() {
            vSemaphoreDelete(_handle);
        }

        bool give(bool yieldFromISR = true) {
#if defined(NRF)
            portBASE_TYPE taskWoken = pdFALSE;
            if(isInterruptHandling()) {
                if(!giveFromIsr()) {
                    return false;
                }
                portEND_SWITCHING_ISR(taskWoken);
                return true;
            }
#endif
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

    inline bool isInterruptHandling() {
        return __get_IPSR() != 0;
    }
}