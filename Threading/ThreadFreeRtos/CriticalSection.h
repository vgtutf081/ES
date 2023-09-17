#pragma once

#include "FreeRTOS.h"
#include "task.h"

namespace ES::Threading {
	class CriticalSection {
	public:
		CriticalSection() {
			_lock = enter();
		}

		~CriticalSection() {
			_lock = exit();
		}		
        
    	private:
		bool enter() {
			taskENTER_CRITICAL();
            return true;
		}

		bool exit() {
			taskEXIT_CRITICAL();
            return false;
        }
		bool _lock = false;
	};
}   