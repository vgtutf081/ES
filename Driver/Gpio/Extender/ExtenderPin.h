#pragma once

#include "IGpio.h"
//#include "PCA9554.h"

namespace ES::Driver::Gpio {

    class ExtenderPin : public IGpio {
    public:
        using Extender = Driver::PCA9554;
        using ID = Extender::ID;
    public:
        ExtenderPin(Extender& extender, ID id) : _extender(extender), _id(id) {

        }

        void set() override {
        }

        void reset() override {
        };

        void toggle() override {
        }

        bool read() override {
        }

        void disable() override {
        }

        void setMode(PinMode mode, DriveMode driveMode, PullMode pullMode) override {

        }

    private:
        Extender& _extender;
        ID _id;
    };

}