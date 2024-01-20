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

    //I met with the extender fall, so I want to get the result without breaking the api
        void set(bool& retSuccess) {
            retSuccess = _extender.set(_id);
            _pinStatus = true;
        }

        void reset(bool& retSuccess) {
            retSuccess = _extender.reset(_id);
            _pinStatus = false;
        }

        void toggle(bool& retSuccess) {
            if(_pinStatus) {
                retSuccess = _extender.reset(_id);
            }
            else {
                retSuccess = _extender.set(_id);
            }
        }

        void set() override {
           bool success = false;
           set(success);
        }

        void reset() override {
            bool success = false;
            reset(success);            
        }

        void toggle() override {
            bool success = false;
            toggle(success);
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