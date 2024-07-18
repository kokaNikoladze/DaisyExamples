#pragma once

#include "daisy_seed.h"
#include "daisysp.h"

class MyDSP
{
  public:
    MyDSP()
    : freq_(440.0f),
      amp_(0.5f),
      smoothFreq_(440.0f),
      smoothAmp_(0.5f),
      sampleRate_(0.0f)
    {
    }

    void Init(float sample_rate)
    {
        sampleRate_ = sample_rate;
        osc_.Init(sample_rate);
        osc_.SetWaveform(daisysp::Oscillator::WAVE_SIN);
        osc_.SetFreq(freq_);
        osc_.SetAmp(amp_);

        SetSmoothingTime(5.0f); // Default 10ms smoothing
    }

    float Process()
    {
        // Smooth frequency and amplitude
        smoothFreq_ += (freq_ - smoothFreq_) * smoothCoeff_;
        smoothAmp_ += (amp_ - smoothAmp_) * smoothCoeff_;

        // Update oscillator with smoothed values
        osc_.SetFreq(smoothFreq_);
        osc_.SetAmp(smoothAmp_);

        return osc_.Process();
    }

    void SetFreq(float freq) { freq_ = freq; }

    void SetAmp(float amp) { amp_ = amp; }

    void SetSmoothingTime(float smoothTimeMs)
    {
        if(sampleRate_ > 0.0f)
        {
            smoothCoeff_ = 1.0f - expf(-1000.0f / (sampleRate_ * smoothTimeMs));
        }
    }

    daisysp::Oscillator osc_;
    float               freq_;
    float               amp_;
    float               smoothFreq_;
    float               smoothAmp_;
    float               smoothCoeff_;
    float               sampleRate_;
};