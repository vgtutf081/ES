#if defined(THREADING)

#include "ThreadFreeRtos.h"


namespace ES::Threading {

    static ID makeThreadId(TaskHandle_t handle) {
        return reinterpret_cast<ID>(handle);
    }

    static ThreadPriority makeThreadPriority(portBASE_TYPE osPriority) {
        return static_cast<ThreadPriority>(osPriority);
    }

    static portBASE_TYPE makeOsPriority(ThreadPriority priority) {
        return static_cast<portBASE_TYPE>(priority);
    }

    ID getId() {
        return makeThreadId(xTaskGetCurrentTaskHandle());
    }
    ThreadPriority getPriority() {
        return makeThreadPriority(uxTaskPriorityGet(xTaskGetCurrentTaskHandle()));
    }
    void setPriority(ThreadPriority priority) {
        vTaskPrioritySet(xTaskGetCurrentTaskHandle(), makeOsPriority(priority));
    }
    void sleepForMs(size_t value){
#if defined(NRF)
        vTaskDelay(value);
#elif defined(CH32V)
        vTaskDelay(value * 1000);
#endif
    }
#if defined(CH32V)
    void sleepForUs(size_t value){
        vTaskDelay(value);
    }
#endif
    void yield(){
        taskYIELD();
    }

}

#endif