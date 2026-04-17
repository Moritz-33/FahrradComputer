#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>

// External TFT object
extern TFT_eSPI tft;

// Display modes
enum DisplayMode {
    STARTUP,
    MAIN_DISPLAY,
    MENU,
    GRAPH
};

// Function declarations
DisplayMode getCurrentMode();
void updateMainDisplay();
void updateDisplayMode();
void showMenu();
void showGraph();
void displayCurrentMode();

#endif // DISPLAY_H