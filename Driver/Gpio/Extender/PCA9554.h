#pragma once

#include "I2CNrf52.h"
#include "Gpio.h"

#include <bitset>
#include <functional>

namespace ES::Driver::Gpio {

class PCA9554x {
    public:
        struct ID {
            static constexpr uint8_t countPins = 8;
            constexpr ID(uint8_t pin) : _pin(pin) {
            }
            
            uint8_t getPin() {
                return _pin;
            }
            private:
            uint8_t _pin;
        };

        enum class PinMode {
            Output = 0,
            Input = 1
        };
        
        enum class Register : uint8_t {
            Input = 0,
            Output,
            Polarity,
            Configuration
        };
        
    public:
        static constexpr std::size_t addresPinsCount = 3;
        PCA9554x(I2C::II2C& i2c, std::bitset<addresPinsCount> addressPinsConfig, uint8_t address) : _i2c(i2c) {
            _address = address;
            for (uint8_t i = 0; i < addressPinsConfig.size(); i++) {
                _address |= addressPinsConfig[i] << i;
            }
            _address = _address << 1;
        }

        bool set(ID pin) {
            return readModifyWriteRegister(static_cast<uint8_t>(Register::Output), [&](uint8_t& value) {
                value &= ~(1 << pin.getPin());
                value |= 1 << pin.getPin();
            });
        }

	    bool reset(ID pin) {
            return readModifyWriteRegister(static_cast<uint8_t>(Register::Output), [&](uint8_t& value) {
                value &= ~(1 << pin.getPin());
                value |= 0 << pin.getPin();
            });
        }

        bool read(ID pin) {
            uint8_t reg = 0;
            _i2c.read(_address, static_cast<uint8_t>(Register::Input), memAddressBitCount, &reg, 1);
            
            bool result = ((reg) & (1 << pin.getPin()));

            return result;
        }

        bool setMode(ID pin, Gpio::PinMode mode) {
            PinMode _extenderPinMode;

            switch (mode) {            
            case Gpio::PinMode::Input :
                _extenderPinMode = PinMode::Input;
                break;
            case Gpio::PinMode::Output :
                _extenderPinMode = PinMode::Output;
                break;
            default:
                return false;
                break;
            }

            return readModifyWriteRegister(static_cast<uint8_t>(Register::Configuration), [&](uint8_t& value) {
                value &= ~(1 << pin.getPin());
                value |= static_cast<bool>(_extenderPinMode) << pin.getPin();
            });
        }
    private:

        bool readModifyWriteRegister(uint8_t address, std::function<void(uint8_t&)> modifyRegister) {
            uint8_t reg = 0;
            bool result = _i2c.read(_address, address, memAddressBitCount, &reg, 1);
            modifyRegister(reg);
            result = _i2c.write(_address, address, memAddressBitCount, &reg, 1);
            return result;
        }
        I2C::II2C& _i2c;

        static constexpr uint16_t memAddressBitCount = 8;
        uint8_t _address;
    };

    class PCA9554 : public PCA9554x {
        static constexpr uint8_t address = 0x20;
    public:
        PCA9554(I2C::II2C& i2c, std::bitset<addresPinsCount> addressPinsConfig) : PCA9554x(i2c, addressPinsConfig, address) {
        }
    };

    class PCA9554A : public PCA9554x {
        static constexpr uint8_t address = 0x38;
    public:
        PCA9554A(I2C::II2C& i2c, std::bitset<addresPinsCount> addressPinsConfig) : PCA9554x(i2c, addressPinsConfig, address) {
        }
    };
}