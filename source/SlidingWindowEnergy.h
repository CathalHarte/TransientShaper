#ifndef SLIDING_WINDOW_ENERGY_H
#define SLIDING_WINDOW_ENERGY_H

#include <vector>
#include <iostream>

class SlidingWindowEnergy {
public:
    SlidingWindowEnergy(int size);

    void addSample(float sample);
    float calculateEnergy() const;

private:
    int size;
    std::vector<float> buffer;
    int index;
    float sum;
};

#endif