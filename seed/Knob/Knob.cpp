#include "daisy_seed.h"
#include "magicFilter.h"

using namespace daisy;

DaisySeed hw;
MidiUsbHandler midi;

constexpr int NUM_ADC_CHANNELS = 12;
constexpr uint8_t PITCH_BEND_STATUS = 0xE0;
constexpr uint8_t Cc = 0xB0;
constexpr uint8_t LSB_MASK = 0x7F;
constexpr uint8_t MSB_SHIFT = 7;
constexpr uint8_t INPUT_TO_12BIT = 4;
constexpr uint8_t UPSCALE_12_TO_14BIT = 2;

uint8_t mode = 0;  // 0: Pitch Bend, 1: NRPN-like CC, 2: Simple 7-bit CC
const uint32_t updateInterval = 1;

Switch button;

magicFilter filters[NUM_ADC_CHANNELS];

void SendNRPNLikeCC(uint8_t baseChannel, uint16_t value) {
    uint8_t lsb = value & LSB_MASK;
    uint8_t msb = (value >> MSB_SHIFT) & LSB_MASK;

    uint8_t ccMessageLSB[3] = {static_cast<uint8_t>(Cc | baseChannel), static_cast<uint8_t>(baseChannel + 1), lsb};
    uint8_t ccMessageMSB[3] = {static_cast<uint8_t>(Cc | baseChannel), static_cast<uint8_t>(baseChannel + 33), msb};

    midi.SendMessage(ccMessageLSB, sizeof(ccMessageLSB));
    midi.SendMessage(ccMessageMSB, sizeof(ccMessageMSB));
}

void SendSimpleCC(uint8_t channel, uint8_t value) {
    uint8_t ccMessage[3] = {static_cast<uint8_t>(Cc | channel), static_cast<uint8_t>(channel + 1), value};
    midi.SendMessage(ccMessage, sizeof(ccMessage));
}

int main(void) {
    hw.Configure();
    hw.Init();
  
    MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midi_cfg);

    AdcChannelConfig adcConfig[NUM_ADC_CHANNELS];
    for (int i = 0; i < NUM_ADC_CHANNELS; i++) {
        adcConfig[i].InitSingle(hw.GetPin(15 + i));
    }
    hw.adc.Init(adcConfig, NUM_ADC_CHANNELS);

    button.Init(hw.GetPin(0), 20, Switch::TYPE_MOMENTARY, Switch::POLARITY_INVERTED, Switch::PULL_UP);

    uint16_t prevValues[NUM_ADC_CHANNELS] = {0};
    uint8_t pitchBendMessage[3] = {PITCH_BEND_STATUS, 0, 0};

    uint32_t lastUpdateTime = System::GetNow();

    while(1) {
        uint32_t currentTime = System::GetNow();
        
        if (currentTime - lastUpdateTime >= updateInterval) {
            lastUpdateTime = currentTime;

            midi.Listen();
            hw.adc.Start();

            button.Debounce();
            if (button.RisingEdge()) {
                mode = (mode + 1) % 3;
            }

            for(int i = 0; i < NUM_ADC_CHANNELS; i++) {
                uint16_t rawValue = hw.adc.Get(i) >> INPUT_TO_12BIT;
                filters[i].update(rawValue);
                uint16_t filteredValue = filters[i].getValue();
                uint16_t value = filteredValue << UPSCALE_12_TO_14BIT;

                if (filters[i].hasChanged() && value != prevValues[i]) {
                    switch(mode) {
                        case 0:
                            pitchBendMessage[0] = PITCH_BEND_STATUS | (i % 16);
                            pitchBendMessage[1] = value & LSB_MASK;
                            pitchBendMessage[2] = (value >> MSB_SHIFT) & LSB_MASK;
                            midi.SendMessage(pitchBendMessage, sizeof(pitchBendMessage));
                            break;
                        case 1:
                            SendNRPNLikeCC(i % 16, value);
                            break;
                        case 2:
                            SendSimpleCC(i % 16, value >> 7);
                            break;
                    }
                    prevValues[i] = value;
                }
            }
        }

        // Yield to system
        // System::Delay(0);
    }
}