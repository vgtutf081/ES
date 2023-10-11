#pragma once

#include "IGpio.h"
#include "PCA9554.h"

namespace ES::Driver::Gpio {

    class ExtenderPin : public IGpio {
    public:
        using Extender = PCA9554x;
        using ID = Extender::ID;
    public:
        ExtenderPin(Extender& extender, ID id) : _extender(extender), _id(id) {

        }

        void set() override {
            _pinStatus = true;
            _extender.set(_id);
        }

        void reset() override {
            _pinStatus = false;
            _extender.reset(_id);
        }

        void toggle() override {
            if(_pinStatus) {
                _extender.reset(_id);
            }
            else {
                _extender.set(_id);
            }
        }

        bool read() override {
            return _extender.read(_id);
        }

        void disable() override {
            
        }

        void setMode(PinMode mode, DriveMode driveMode, PullMode pullMode) override {
            _extender.setMode(_id, mode);
        }

    private:
        bool _pinStatus = false;

        Extender& _extender;
        ID _id;
    };

}