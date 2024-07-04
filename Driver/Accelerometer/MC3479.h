#pragma once

#include "II2C.h"
#include <cstdint>
#include "IGpio.h"
#include <array>

namespace ES::Driver::Accelerometer {
    class MC3479 {
    public:
        static constexpr uint8_t WriteAddressA6High = 0xD8;
        static constexpr uint8_t WriteAddressA6Low = 0x98;
        static constexpr uint8_t ReadAddressA6High = 0xD9;
        static constexpr uint8_t ReadAddressA6Low = 0x99;

        enum class Registers : uint8_t {
            DeviceStatus = 0x05,
            InterruptEnable = 0x06,
            Mode = 0x07,
            SampleRate = 0x08,
            MotionControl = 0x09,
            FIFOStatusRegister = 0x0A,
            FIFOReadPointer = 0x0B,
            FIFOWritePointer = 0x0C,
            XAccelerometerDataLSB = 0x0D,
            XAccelerometerDataMSB = 0x0E,
            YAccelerometerDataLSB = 0x0F,
            YAccelerometerDataMSB = 0x10,
            ZAccelerometerDataLSB = 0x11,
            ZAccelerometerDataMSB = 0x12,
            Status = 0x13,
            InterruptStatus = 0x14,
            ChipID = 0x18,
            RangeSelectorControl = 0x20,
            XOffsetLSB = 0x21,
            XOffsetMSB = 0x22,
            YOffsetLSB = 0x23,
            YOffsetMSB = 0x24,
            ZOffsetLSB = 0x25,
            ZOffsetMSB = 0x26,
            XGain = 0x27,
            YGain = 0x28,
            ZGain = 0x29,
            FIFOControl = 0x2D,
            FIFOThershold = 0x2E,
            FIFOInterruptStatus = 0x2F,
            FIFOControl2 = 0x30,
            CommControl = 0x31,
            GPIOControl = 0x33,
            TiltFlipThresholdLSB = 0x40,
            TiltFlipThresholdMSB = 0x41,
            TiltFlipDebounce = 0x42,
            AnyMotionThresholdLSB = 0x43,
            AnyMotionThresholdMSB = 0x44,
            AnyMotionDebounce = 0x45,
            ShakeThresholdLSB = 0x46,
            ShakeThresholdMSB = 0x47,
            PeakToPeakDurationLSB = 0x48,
            PeakToPeakDurationMSB = 0x49,
            TimerControl = 0x4A,
            ReadCounter = 0x4B,
        };
#pragma pack(push, 1)
        struct DeviceStatusRegister {
            bool oneTimeProg : 1;
            uint8_t reserved2 : 2;
            bool i2cWatchdog : 1;
            uint8_t reserved1 : 1;
            bool resolutionMode : 1;
            uint8_t state : 2;
        };

        struct InterruptEnableRegister {
            bool generateInterrupt : 1;
            bool autoClearenable : 1;
            uint8_t reserved : 1;
            bool tilt35Enable : 1;
            bool shakeEnable : 1;
            bool anyMotionEnable : 1;
            bool flipEnable : 1;
            bool tiltEnable : 1;
        };

        struct ModeRegister {
            uint8_t reserved : 2;
            bool watchdogPositive : 1;
            bool watchdogNegative : 1;
            const uint8_t zero : 2;
            uint8_t state : 2;
        };

        struct SampleRateRegister {
            uint8_t zero : 3;
            uint8_t rate : 5;
        };

        struct MotionControlRegister {
            bool motionReset : 1;
            bool rawDataFiltering : 1;
            bool zAxisOrientation : 1;
            bool tilt35Enable : 1;
            bool shakeEnable : 1;
            bool anyMotionEnable : 1;
            bool motionLatch : 1;
            bool tiltFlipEnable : 1;
        };

        struct FIFOStatusRegister {
            uint8_t reserved : 5;
            bool fifoThreshold : 1;
            bool fifoFull : 1;
            bool fifoEmpty : 1;
        };

        struct DataAccelerometer {
            int16_t zData;
            int16_t yData;
            int16_t xData;
        };

        struct StatusRegister {
            bool newData : 1;
            uint8_t reserved : 1;
            bool fifoFlag : 1;
            bool tilt35flag : 1;
            bool shakeFlag : 1;
            bool anyMotionFlag : 1;
            bool flipFlag : 1;
            bool tiltFlag : 1;
        };

        struct InterruptStatusRegister {
            bool generateInterrupt : 1;
            uint8_t reserved : 1;
            bool fifoInterrupt : 1;
            bool tilt35Interrupt : 1;
            bool shakeInterrupt : 1;
            bool anyMotionInterrupt : 1;
            bool flipInterrupt : 1;
            bool tiltInterrupt : 1;
        };
#pragma pack(pop)        

        MC3479(I2C::II2C& i2c, Gpio::IGpio& int1, Gpio::IGpio& int2, bool A6LogicLevel) : _i2c(i2c), _int1(int1), _int2(int2) {
            //_int1.configureInput(Gpio::PullMode::Up);
            //_int2.configureInput(Gpio::PullMode::Up);

            if(A6LogicLevel) {
                _readAddress = ReadAddressA6High;
                _writeAddress = WriteAddressA6High;
            }
            else {
                _readAddress = ReadAddressA6Low;
                _writeAddress = WriteAddressA6Low;
            }
        }

        DeviceStatusRegister* readDeviceStatusRegister() {
            return readReg<DeviceStatusRegister>(Registers::DeviceStatus);
        }

        void writeMode(ModeRegister reg) {
            writeReg(reg, Registers::Mode);
        }

        ModeRegister* readMode() {
            return readReg<ModeRegister>(Registers::Mode);
        }

        void wake() {
            auto mode = readMode();
            mode->state = 1;
            writeMode(*mode);
        }

        void standby() {
            auto mode = readMode();
            mode->state = 0;
            writeMode(*mode);
        }

        void writeInterruptEnable(InterruptEnableRegister reg) {
            writeReg(reg, Registers::InterruptEnable);
        }

        void writeMotionControl(MotionControlRegister reg) {
            writeReg(reg, Registers::MotionControl);
        }

        DataAccelerometer* readData() {
            return readReg<DataAccelerometer>(Registers::XAccelerometerDataLSB);
        }

        StatusRegister* readStatus() {
            return readReg<StatusRegister>(Registers::Status);
        }

        InterruptStatusRegister* readInterruptStatus() {
            return readReg<InterruptStatusRegister>(Registers::InterruptStatus);
        }

        void writeInterruptStatus(InterruptStatusRegister reg) {
            writeReg(reg, Registers::InterruptStatus);
        }

    protected:
        uint8_t _buf[8];
        uint8_t _readAddress = 0;
        uint8_t _writeAddress = 0;
        Gpio::IGpio& _int2;
        Gpio::IGpio& _int1;
        I2C::II2C& _i2c;

        template <typename RegType> void writeReg(RegType reg, Registers regAddress) {
            _i2c.write(_writeAddress, static_cast<uint8_t>(regAddress), 1, reinterpret_cast<uint8_t*>(&reg), sizeof(reg));
        }

        template <typename RegType> RegType* readReg(Registers regAddress) {
            _i2c.read(_readAddress, static_cast<uint8_t>(regAddress), 1, _buf, sizeof(RegType));
            return reinterpret_cast<RegType *>(_buf);
        }
    };
}