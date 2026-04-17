#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <Arduino.h>

// Pin definitions
#define KNOPF1_PIN 35
#define KNOPF2_PIN 0
#define MAGNET_PIN 33

enum button1Mode {
    DISPLAY_MODE_SELECTOR,
    MENU_NAVIGATOR,
    GRAPH_TIME_SELECTOR
};

enum button2Mode {
    SLEEP_REQUESTER,
    MENU_SELECTOR
};
// Global flag for sleep request
extern bool sleepRequested;
void setupInterrupts();
void isrKnopf1();
void isrKnopf2();
void isrMagnet();

extern volatile unsigned long lastDistanceUpdateTime;


#endif // INTERRUPTS_H