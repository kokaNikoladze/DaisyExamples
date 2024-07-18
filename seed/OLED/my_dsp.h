#pragma once

#include "daisy_seed.h"
#include "daisysp.h"

class MyDSP {
public:
    MyDSP() : hw_() {}

    void Init() {
        hw_.Configure();
        hw_.Init();
        hw_.SetAudioBlockSize(4); // Set your desired block size
        float sample_rate = hw_.AudioSampleRate();

        osc_.Init(sample_rate);
        osc_.SetWaveform(daisysp::Oscillator::WAVE_SIN);
        osc_.SetFreq(440.0f);
        osc_.SetAmp(0.5f);
        // Initialize any other DSP modules here
    }

    void Start() {
        hw_.StartAudio(AudioCallback);
    }

    static void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t size) {
        for (size_t i = 0; i < size; i++) {
            float sample = instance_->osc_.Process();
            out[0][i] = out[1][i] = sample;
        }
    }

    void SetFrequency(float freq) {
        osc_.SetFreq(freq);
    }

    // Add any other methods you need

    static void SetInstance(MyDSP* instance) { instance_ = instance; }

private:
    daisy::DaisySeed hw_;
    daisysp::Oscillator osc_;
    static MyDSP* instance_;
    // Add any other DSP modules or variables you need
};

// Define the static member variable
MyDSP* MyDSP::instance_ = nullptr;