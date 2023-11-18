#pragma once

#include "IGpio.h"
#include "PCA9554.h"

namespace ES::Driver::Gpio {

    class ExtenderPin : public IGpio {
    public:
        using Extender = PCA9554x;
        using ID = Extender::ID;
    public:
        constexpr ExtenderPin(Extender& extender, ID id) : _extender(extender), _id(id) {

        }

        void set() override {
            _extender.set(_id);
            _pinStatus = true;
        }

        void reset() override {
            _extender.reset(_id);
            _pinStatus = false;
        };

        void toggle() override {
            if(_pinStatus) {
                _extender.reset(_id);
            }
            else {
                _extender.set(_id);
            }
        }

        bool read() const override {
            return _extender.read(_id);
        }

        void disable() override {

        }

        uint32_t getPin() const override {
            return 0;
        }

        uint32_t getPort() const override {
            return 0;
        }

        uint32_t getPortAndPin() const override {
            return 0;
        }

        void setMode(PinMode mode) override {
            _extender.setMode(_id, mode);
        }

        void setPullMode(PullMode mode) override {
        }

        void setDriveMode(DriveMode mode) override {
        }


    private:
        bool _pinStatus = false;

        Extender& _extender;
        ID _id;
    };

}