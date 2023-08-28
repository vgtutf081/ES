#pragma once
#include <functional>
#include <cstddef>
#include <cstdint>

#include "FreeRTOS.h"
#include "task.h"

namespace ES::Threading::ThreadFreeRtos {
    
    class Thread {
    public:
        explicit Thread(const char* name, uint16_t stackSize, uint32_t priority, std::function<void()>&& body) : _body(std::move(body)) {
            auto status = xTaskCreate(staticTaskHandler, name, stackSize, this, (UBaseType_t)priority, &_taskHandle);
    
        }

        Thread(Thread&& other) {
        }

        Thread() {
        }

        ~Thread() {
            vTaskDelete(_taskHandle);
        }

        private:
        /*static void staticTaskHandler(void* param){
		    Thread* _this = (Thread*)param;
		    _this->taskHandler();
	    }*/

        static void staticTaskHandler(void * pvParameter) {
		    Thread* _this = (Thread*)pvParameter;
		    _this->taskHandler();
        }

        void taskHandler(){
            _body();
            vTaskDelete(xTaskGetCurrentTaskHandle());
	    }

        void suspend() {
            vTaskSuspend(_taskHandle);
        }
        void resume() {
            vTaskResume(_taskHandle);
        }
        
        UBaseType_t getPriority() const {
            return uxTaskPriorityGet(_taskHandle);
        }
        void setPriority(UBaseType_t priority) const {
            return vTaskPrioritySet(_taskHandle, priority);
        }

        TaskHandle_t _taskHandle = nullptr;
        std::function<void()> _body;
    };
}