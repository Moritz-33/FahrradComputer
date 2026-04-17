#ifndef METRICS_H
#define METRICS_H

#include <Arduino.h>

// Global variables
extern volatile double km;
extern volatile double speed; 
extern const double weg;
extern volatile unsigned long pulseIntervalMs; 
extern volatile unsigned long lastPulseTime;
extern volatile int lastSpeedUpdateTime;
extern volatile int uptimeSeconds;
extern volatile int uptimeMinutes;
extern volatile int uptimeHours;


// Function declarations
void tagkm();
void resetKm();
void calculateSpeed();
void updateUptime();

// Graph history functions
void sampleSpeedHistory();
void setGraphTimeframeSeconds(uint8_t seconds);
uint8_t getGraphTimeframeSeconds();
void getSpeedGraphData(double* dest, uint8_t& count);

#endif // METRICS_H