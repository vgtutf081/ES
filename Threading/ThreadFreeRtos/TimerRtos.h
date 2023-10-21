#pragma once

#include "FreeRTOS.h"
#include "timers.h"

#include "GpioCh32v.h"

namespace ES::Threading {
    class TimerRtos {
    public:
        TimerRtos(const char * const timerName, TickType_t periodMs, UBaseType_t autoReload) {
            _timerID = this;
            _handle = xTimerCreate(timerName, periodMs * 100, autoReload, _timerID, timerCallBack);
        }

        static void timerCallBack(TimerHandle_t timer) {
            auto ptr = reinterpret_cast<TimerRtos*>(pvTimerGetTimerID(timer));
        }

        TimerHandle_t getHandle() {
            return _handle;
        }

        void start(TickType_t wait = 0) {
            xTimerStart(_handle, portMAX_DELAY);
        }

        void stop(TickType_t wait = 0) {
            xTimerStop(_handle, portMAX_DELAY);
        }

    private:

        TimerHandle_t _handle;
        void * _timerID;
    };
    

    class StopWatch {
    public: 
        StopWatch() {
            
        }

        void start() {
            _startCount = xTaskGetTickCount();
            _isStarted = true;
        }
        
        void stop() {
            _endCount = xTaskGetTickCount();

            if(_endCount > _startCount) {
                _timeUs = ((portMAX_DELAY - _startCount) + _endCount) * 10;
            }
            else if(_startCount == _endCount) {
                _timeUs = 0;
            }
            else {
                _timeUs =  (_endCount - _startCount) * 10;
            }
            _isStarted = false;
        }

        bool isStarted() {
            return _isStarted;
        }

        uint32_t getTimeUs() {
            return _timeUs;
        }

        private:
        bool _isStarted = false;
        TickType_t _startCount;
        TickType_t _endCount;

        uint32_t _timeUs = 0;
    };
}

