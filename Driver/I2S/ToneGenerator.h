#pragma once 

#include <cmath>
#include <vector>
#include "I2SNrf52.h"

namespace ES::Driver::Tone {
    class ToneGenerator {
    public:
        enum Tone {
            Sine,
            SawTooth,
            Triangle
        };

        ToneGenerator() {

        }

        const std::vector<uint32_t>& generateTone(Tone tone, uint32_t freq, float amplitude, uint32_t offset, I2S::SampleRate sampleRate) {
            float sampleTime = 1000000 / static_cast<float>(sampleRate);
            float seqTime = 1000000 / freq;
            size_t PartSize = seqTime / sampleTime;
            if(tone == Tone::Sine) {
                float angle = 0.f;
                uint32_t a = offset * amplitude;
                for(size_t i = 0; i < PartSize; i++) {
                    vec.push_back(uint32_t((a * sin(angle)) + offset));
                    angle += (2.f * (float)M_PI) / PartSize;
                }
            }
            return vec;
        }

    private:
        std::vector<uint32_t> vec;
    };
}