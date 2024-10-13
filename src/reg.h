#ifndef REG_H
#define REG_H

#include "Arduino.h"

class LinearRegression {
public:
    static float calculateSlope(const float* x, const float* y, size_t size);
    static float calculateIntercept(const float* x, const float* y, size_t size);
    static void calculateRegression(const float* x, const float* y, size_t size, float& slope, float& intercept);
};

#endif // REG_H
