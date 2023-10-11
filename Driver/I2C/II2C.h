#pragma once

namespace ES::Driver::I2C {
    class II2C {
        public:
        virtual ~II2C() = default;

        virtual bool read(uint16_t address,  uint8_t* buffer, size_t size) = 0;
        virtual bool read(uint16_t address, std::uint16_t memAddress, size_t memAddressBitCount, uint8_t* buffer, size_t size) = 0;
        virtual bool write(uint16_t address, const uint8_t* buffer, size_t size) = 0;
        virtual bool write(uint16_t address, std::uint16_t memAddress, size_t memAddressBitCount, const uint8_t* buffer, size_t size) = 0;
    };

}