#include "startup.h"
#include <Arduino.h>
#include "display.h"

void setupDisplay(TFT_eSPI& tft) {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
}

void showStartupScreen(TFT_eSPI& tft) {
    tft.drawString("Fahrradcomputer", 0, 50, 4);
    
    // Progress bar animation for 5 seconds
    int barY = 100;
    int barHeight = 10;
    int barWidth = tft.width() - 20; // margin
    int barX = 10;
    
    // Draw outline
    tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
    
    // Animate progress bar over 5 seconds (50 steps, 100ms each)
    for(int i = 0; i <= 50; i++) {
        int fillWidth = (i / 50.0) * (barWidth - 4);
        tft.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, TFT_WHITE);
        delay(100);
    }
    updateDisplayMode();
}