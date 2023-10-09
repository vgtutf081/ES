#pragma once

namespace ES::Driver::I2C {
    class II2C {
        public:
        virtual ~II2C() = default;

        virtual bool read(uint16_t address, void* buffer, size_t size) = 0;
        virtual bool write(uint16_t address, const void* buffer, size_t size) = 0;
        virtual bool writeAsync(uint16_t address, I2CWriteTransfer* transfer) {  return false; }
    };

}