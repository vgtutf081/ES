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
            size_t PartSize = static_cast<float>(sampleRate) / freq;
            float fixedAmplitude = offset * amplitude;
            float value = 0.0f;
            vec.clear();
            for (size_t frame = 0; frame < PartSize; ++frame) {
                float pos = fmod(freq * frame / static_cast<float>(sampleRate), 1.0f);
                if (tone == Tone::Sine) {
                    value = sin(pos * 2 * M_PI);
                }
                else if (tone == Tone::SawTooth) {
                    value = pos * 2 - 1;
                }
                else if (tone == Tone::Triangle) {
                    value = 1 - fabs(pos - 0.5) * 4;
                }
                vec.push_back(static_cast<uint32_t>(value * fixedAmplitude + offset));
            }
            return vec;
        }

    private:
        std::vector<uint32_t> vec;
    };
}