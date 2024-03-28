#pragma once

#include <cstdint>

namespace ES::Driver {
    struct ISpiEventHandler {
        virtual ~ISpiEventHandler() = default;
        virtual void onTransferComplete() = 0;
    };

    struct ISpiMaster {
        virtual ~ISpiMaster() = default;
        virtual void setEventHandler(ISpiEventHandler* handler) = 0;
        virtual void read(uint8_t* rxBuffer, size_t size) = 0;
        virtual void write(const uint8_t* txBuffer, size_t size) = 0;
        virtual bool readWrite(const uint8_t* txBuffer, uint8_t* rxBuffer, size_t size) = 0;
    };
}