#include "metrics.h"

volatile double km = 0; // global distance in km
volatile double speed = 0; // global speed in km/h
const double weg = 2.15; // distance per pulse in m
volatile unsigned long pulseIntervalMs = 0;
volatile unsigned long lastPulseTime = 0;
volatile int lastSpeedUpdateTime = 0;
volatile int uptimeSeconds = 0;
volatile int uptimeMinutes = 0;
volatile int uptimeHours = 0;
double wegInKm = 0.00215;

static const uint8_t GRAPH_HISTORY_MAX = 120;
static double speedHistory[GRAPH_HISTORY_MAX];
static uint8_t speedHistoryCount = 0;
static uint8_t speedHistoryPos = 0;
static uint8_t graphTimeframeSeconds = 30;

double getClampedSpeed(double value) {
    return value < 0 ? 0 : value;
}

void tagkm() {
    km += wegInKm;
}

void resetKm() {
    
}

void calculateSpeed() {
    unsigned long currentTime = millis();
    lastSpeedUpdateTime = currentTime;
    pulseIntervalMs = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
    speed = (weg / (pulseIntervalMs / 1000.0)) * 3.6;
}

void sampleSpeedHistory() {
    speedHistory[speedHistoryPos] = getClampedSpeed(speed);
    if (speedHistoryCount < GRAPH_HISTORY_MAX) {
        speedHistoryCount++;
    }
    speedHistoryPos = (speedHistoryPos + 1) % GRAPH_HISTORY_MAX;
}

void setGraphTimeframeSeconds(uint8_t seconds) {
    if (seconds == 0) {
        seconds = 30;
    }
    graphTimeframeSeconds = (seconds > GRAPH_HISTORY_MAX ? GRAPH_HISTORY_MAX : seconds);
}

uint8_t getGraphTimeframeSeconds() {
    return graphTimeframeSeconds;
}

void getSpeedGraphData(double* dest, uint8_t& count) {
    count = (speedHistoryCount < graphTimeframeSeconds ? speedHistoryCount : graphTimeframeSeconds);
    if (count == 0) {
        return;
    }

    uint8_t startIndex = 0;
    if (speedHistoryCount >= GRAPH_HISTORY_MAX) {
        startIndex = speedHistoryPos;
    }
    if (speedHistoryCount > graphTimeframeSeconds) {
        startIndex = (speedHistoryPos + GRAPH_HISTORY_MAX - count) % GRAPH_HISTORY_MAX;
    }

    for (uint8_t i = 0; i < count; i++) {
        dest[i] = speedHistory[(startIndex + i) % GRAPH_HISTORY_MAX];
    }
}

void updateUptime() {
    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 1000) {
        uptimeSeconds = uptimeSeconds + 1;
        if (uptimeSeconds >= 60) {
            uptimeSeconds = 0;
            uptimeMinutes = uptimeMinutes + 1;
        }
        if (uptimeMinutes >= 60) {
            uptimeMinutes = 0;
            uptimeHours = uptimeHours + 1;
        }
        lastUpdateTime = currentTime;
        sampleSpeedHistory();
    }
}
