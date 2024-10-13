#include "reg.h"

float LinearRegression::calculateSlope(const float* x, const float* y, size_t size) {
    size_t i = 0;
    float sumX = 0.0f, sumY = 0.0f, sumXY = 0.0f, sumX2 = 0.0f;
    
    // Unrolling manually for every 2nd iteration
    for (; i + 1 < size; i += 2) {
        float x0 = x[i], x1 = x[i + 1];
        float y0 = y[i], y1 = y[i + 1];
        
        sumX += x0 + x1;
        sumY += y0 + y1;
        sumXY += x0 * y0 + x1 * y1;
        sumX2 += x0 * x0 + x1 * x1;
    }

    // Handle remaining iteration
    for (; i < size; ++i) {
        float xVal = x[i];
        float yVal = y[i];
        
        sumX += xVal;
        sumY += yVal;
        sumXY += xVal * yVal;
        sumX2 += xVal * xVal;
    }

    float numerator = (float)size * sumXY - sumX * sumY;
    float denominator = (float)size * sumX2 - sumX * sumX;

    // Precompute the result for division
    if (denominator == 0.0f) return 0.0f;

    return numerator / denominator;
}

float LinearRegression::calculateIntercept(const float* x, const float* y, size_t size) {
    size_t i = 0;
    float sumX = 0.0f, sumY = 0.0f;

    // Unrolling manually for every 2nd iteration
    for (; i + 1 < size; i += 2) {
        sumX += x[i] + x[i + 1];
        sumY += y[i] + y[i + 1];
    }

    // Handle remaining iteration
    for (; i < size; ++i) {
        sumX += x[i];
        sumY += y[i];
    }

    float meanX = sumX / size;
    float meanY = sumY / size;

    // Precompute slope to avoid redundant calculations
    float slope = calculateSlope(x, y, size);

    // Inline return computation to minimize function calls
    return meanY - slope * meanX;
}

void LinearRegression::calculateRegression(const float* x, const float* y, size_t size, float& slope, float& intercept) {
    // Call inlined methods to avoid separate computation
    slope = calculateSlope(x, y, size);
    intercept = calculateIntercept(x, y, size);
}
