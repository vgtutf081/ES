#pragma once 

#include "CriticalSection.h"
#include "IGpio.h"

namespace ES::Driver::Gpio {

    struct InterruptCallback {
		/*virtual ~InterruptCallback(){
			releaseInterrupt(this);
		}*/
		virtual void onGpioInterrupt(uint32_t pin) = 0;
	};

    struct InterruptCallback;
    void configureInterrupt(IGpio& pin, PullMode pullMode, InterruptMode interruptMode, InterruptCallback* callback);
    void releaseInterrupt(InterruptCallback* callback);
    void releaseInterrupt(IGpio& pin);
}