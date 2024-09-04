#include "SlidingWindowEnergy.h"
#include <stdexcept>  // For exception handling

SlidingWindowEnergy::SlidingWindowEnergy(int size) : size(size), sum(0.0f), index(0), buffer(size, 0.0f) {
    if (size <= 0) {
        throw std::invalid_argument("Size of the buffer must be greater than zero.");
    }
    std::cout << "Initialized SlidingWindowEnergy with size: " << size << std::endl;
}

void SlidingWindowEnergy::addSample(float sample) {

    float oldSample = buffer[index];
    sum = sum - (oldSample * oldSample) + (sample * sample);

    buffer[index] = sample;
    index = (index + 1) % size;
}

float SlidingWindowEnergy::calculateEnergy() const {
    return sum / size;
}
