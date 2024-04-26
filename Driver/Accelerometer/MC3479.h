#pragma once

#include "II2C.h"
#include <cstdint>
#include "IGpio.h"
#include <array>

namespace ES::Driver::Accelerometer {
    class MC3479 {
    public:
        static constexpr uint8_t AddressA6High = 0xD8;
        static constexpr uint8_t AddressA6Low = 0x98;

        enum class Registers : uint8_t {
            DeviceStatusRegister = 0x05,
            InterruptEnableRegister = 0x06
        };

        MC3479(I2C::II2C& i2c, Gpio::IGpio& int1, Gpio::IGpio& int2, bool A6LogicLevel) : _i2c(i2c), _int1(int1), _int2(int2) {
            //_int1.configureInput(Gpio::PullMode::Up);
            //_int2.configureInput(Gpio::PullMode::Up);

            if(A6LogicLevel) {
                _address = AddressA6High;
            }
            else {
                _address = AddressA6Low;
            }
        }

        uint8_t readStatusRegister() {

            _i2c.read(_address, static_cast<uint8_t>(Registers::DeviceStatusRegister), 1,_buf, 1);
            return _buf[0];
        }

    protected:
        uint8_t _buf[1];
        uint8_t _address = 0;
        Gpio::IGpio& _int2;
        Gpio::IGpio& _int1;
        I2C::II2C& _i2c;
    };
}