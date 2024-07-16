// ResponsiveAnalogRead made into a magicFilter

#pragma once

#include <cmath>

class magicFilter {
public:
    magicFilter(bool sleepEnable = true, float snapMultiplier = 0.01)
        : sleepEnable(sleepEnable), snapMultiplier(snapMultiplier),
          analogResolution(4096), activityThreshold(8),
          edgeSnapEnable(true), smoothValue(0), errorEMA(0), rawValue(0),
          magicValue(0), prevMagicValue(0), sleeping(false) {}

    void update(int newValue) {
        rawValue = newValue;
        prevMagicValue = magicValue;
        magicValue = getMagicValue(newValue);
    }

    int getValue() { return magicValue; }
    bool hasChanged() { return magicValue != prevMagicValue; }

private:
    int getMagicValue(int newValue) {
        if (sleepEnable && edgeSnapEnable) {
            if (newValue < activityThreshold) {
                newValue = (newValue * 2) - activityThreshold;
            } else if (newValue > analogResolution - activityThreshold) {
                newValue = (newValue * 2) - analogResolution + activityThreshold;
            }
        }

        unsigned int diff = abs(newValue - smoothValue);

        errorEMA += ((newValue - smoothValue) - errorEMA) * 0.4;

        if (sleepEnable) {
            sleeping = abs(errorEMA) < activityThreshold;
        }

        if (sleepEnable && sleeping) {
            return (int)smoothValue;
        }

        float snap = snapCurve(diff * snapMultiplier);

        if (sleepEnable) {
            snap *= 0.5 + 0.5;
        }

        smoothValue += (newValue - smoothValue) * snap;

        if (smoothValue < 0.0) {
            smoothValue = 0.0;
        } else if (smoothValue > analogResolution - 1) {
            smoothValue = analogResolution - 1;
        }

        return (int)smoothValue;
    }

    float snapCurve(float x) {
        float y = 1.0 / (x + 1.0);
        y = (1.0 - y) * 2.0;
        return (y > 1.0) ? 1.0 : y;
    }

    bool sleepEnable;
    float snapMultiplier;
    int analogResolution;
    int activityThreshold;
    bool edgeSnapEnable;
    float smoothValue;
    float errorEMA;
    int rawValue;
    int magicValue;
    int prevMagicValue;
    bool sleeping;
};