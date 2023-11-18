#pragma once

#include <atomic>
#include "ThreadFreeRtos.h"

namespace ES::Threading {
    class ActionLock {
    public:
        bool tryLock() {
            return !_locker.test_and_set(std::memory_order_acquire);
        }
        void lock() {
            while(!tryLock()){
                yield();
            }
        }
        void unlock() {
            _locker.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag _locker = ATOMIC_FLAG_INIT;
    };
}