#ifndef GRAPH_H
#define GRAPH_H

#include <Arduino.h>

enum GraphOptions {
    OPTION_30S,
    OPTION_60S,
    OPTION_120S
};

void buildGraphPoints(const double* samples, uint8_t sampleCount, int left, int right, int top, int bottom, int* xPoints, int* yPoints, double& maxSpeed);

double getGraphMaxSpeed(const double* samples, uint8_t sampleCount);

#endif // GRAPH_H