#include "graph.h"

void buildGraphPoints(const double* samples, uint8_t sampleCount, int left, int right, int top, int bottom, int* xPoints, int* yPoints, double& maxSpeed) {
    int graphWidth = right - left;
    int graphHeight = bottom - top;

    maxSpeed = 10;
    for (uint8_t i = 0; i < sampleCount; i++) {
        if (samples[i] > maxSpeed) {
            maxSpeed = samples[i];
        }
    }

    maxSpeed += 2;
    if (maxSpeed < 10) {
        maxSpeed = 10;
    }

    for (uint8_t i = 0; i < sampleCount; i++) {
        xPoints[i] = left + (sampleCount == 1 ? graphWidth / 2 : (graphWidth * i) / (sampleCount - 1));
        double normalized = (samples[i] / maxSpeed);
        if (normalized < 0) {
            normalized = 0;
        }
        if (normalized > 1) {
            normalized = 1;
        }
        yPoints[i] = bottom - (int)(normalized * graphHeight);
    }
}

double getGraphMaxSpeed(const double* samples, uint8_t sampleCount) {
    double maxSpeed = 10;
    for (uint8_t i = 0; i < sampleCount; i++) {
        if (samples[i] > maxSpeed) {
            maxSpeed = samples[i];
        }
    }
    maxSpeed += 2;
    if (maxSpeed < 10) {
        maxSpeed = 10;
    }
    return maxSpeed;
}
