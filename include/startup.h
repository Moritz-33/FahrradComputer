#ifndef STARTUP_H
#define STARTUP_H

#include <TFT_eSPI.h>

// Function declarations
void setupDisplay(TFT_eSPI& tft);
void showStartupScreen(TFT_eSPI& tft);

#endif // STARTUP_H