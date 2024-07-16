# Knob

## Author

Koka

This is a working example of a MIDI controller code, responsiveAnalogreading ADC values from all 12 analog inputs and sending them over usb midi. A push button cycles through three different modes:

1. PitchBend sending on 12 individual channels (12-bit usable readings upscaled to 14 bits);
2. NRPN (split between channels 1-33, 2-34, etc. 12-bit value upscaled to 14 bits);
3. Normal 7-bit midi cc.
