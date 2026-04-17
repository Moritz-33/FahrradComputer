#include "display.h"
#include "metrics.h"
#include "startup.h"
#include "graph.h"

extern GraphOptions GraphOptionSelection;
extern bool graphResolutionSubmenuActive;

DisplayMode currentMode = STARTUP;

void updateDisplayMode()
{
    switch (currentMode)
    {
    case STARTUP:
        currentMode = MAIN_DISPLAY;
        break;
    case MAIN_DISPLAY:
        currentMode = GRAPH;
        break;
    case GRAPH:
        currentMode = MENU;
        break;
    case MENU:
        currentMode = MAIN_DISPLAY;
        break;
    }
}

void displayCurrentMode()
{
    switch (currentMode)
    {
    case STARTUP:
        showStartupScreen(tft);
        break;
    case MAIN_DISPLAY:
        updateMainDisplay();
        break;
    case GRAPH:
        showGraph();
        break;
    case MENU:
        showMenu();
        break;
    }
}


void updateMainDisplay()
{
    tft.fillScreen(TFT_BLACK);
    
    int centerX = tft.width() / 2;
    // Draw vertical separating line
    tft.drawLine(centerX, 0, centerX, 80, TFT_WHITE);
    
    // Draw Horizontal line
    tft.drawLine(0, 80, tft.width(), 80, TFT_WHITE);
    
    // Left side: Speed
    int leftCenterX = tft.width() / 4;
    String speedStr = String(speed, 2);
    int speedW = tft.textWidth(speedStr, 4);
    int speedX = leftCenterX - speedW / 2;
    tft.drawString(speedStr, speedX, 20, 4);
    
    String speedUnit = "km/h";
    int speedUnitW = tft.textWidth(speedUnit, 4);
    int speedUnitX = leftCenterX - speedUnitW / 2;
    tft.drawString(speedUnit, speedUnitX, 50, 4);

    // Right side: Distance
    int rightCenterX = tft.width() * 3 / 4;
    String distStr = String(km, 3);
    int distW = tft.textWidth(distStr, 4);
    int distX = rightCenterX - distW / 2;
    tft.drawString(distStr, distX, 20, 4);

    String distUnit = "km";
    int distUnitW = tft.textWidth(distUnit, 4);
    int distUnitX = rightCenterX - distUnitW / 2;
    tft.drawString(distUnit, distUnitX, 50, 4);

    String uptimeStr = String(uptimeHours) + ":" + (uptimeMinutes < 10 ? "0" : "") + String(uptimeMinutes) + ":" + (uptimeSeconds < 10 ? "0" : "") + String(uptimeSeconds);
    int uptimeW = tft.textWidth(uptimeStr, 4);
    int uptimeX = centerX - uptimeW / 2;
    tft.drawString(uptimeStr, uptimeX, 100, 4);
}

void showMenu()
{
    tft.fillScreen(TFT_BLACK);
    if (!graphResolutionSubmenuActive) {
        tft.drawString("Menu", 0, 20, 4);
    }
    
    if (!graphResolutionSubmenuActive) {
        tft.drawString("> Graph resolution", 10, 60, 2);
        return;
    }else {
        tft.drawString("Graph resolution", 0, 20, 4);
    }

    int yPos = 60;
    int lineHeight = 24;

    if (GraphOptionSelection == OPTION_30S) {
        tft.drawString("> 30s", 10, yPos, 2);
    } else {
        tft.drawString("  30s", 10, yPos, 2);
    }
    yPos += lineHeight;

    if (GraphOptionSelection == OPTION_60S) {
        tft.drawString("> 60s", 10, yPos, 2);
    } else {
        tft.drawString("  60s", 10, yPos, 2);
    }
    yPos += lineHeight;

    if (GraphOptionSelection == OPTION_120S) {
        tft.drawString("> 120s", 10, yPos, 2);
    } else {
        tft.drawString("  120s", 10, yPos, 2);
    }
    yPos += lineHeight;
    tft.drawString("Press Button 2 to save", 10, yPos, 2);
}

void showGraph()
{
    tft.fillScreen(TFT_BLACK);

    const uint8_t MAX_GRAPH_POINTS = 120;
    double samples[MAX_GRAPH_POINTS];
    uint8_t sampleCount = 0;
    getSpeedGraphData(samples, sampleCount);
    uint8_t timeframe = getGraphTimeframeSeconds();

    int left = 10;
    int right = tft.width() - 10;
    int top = 20;
    int bottom = tft.height() - 40;
    int graphWidth = right - left;
    int graphHeight = bottom - top;

    // Graph header
    String title = "Speed history ";
    title += String(timeframe);
    title += "s";
    tft.drawString(title, left, 0, 2);

    // Draw graph frame
    tft.drawRect(left, top, graphWidth, graphHeight, TFT_WHITE);

    if (sampleCount == 0) {
        tft.drawString("No speed data yet", left + 10, top + graphHeight / 2 - 10, 2);
        return;
    }

    double maxSpeed = getGraphMaxSpeed(samples, sampleCount);
    int xPoints[120];
    int yPoints[120];
    buildGraphPoints(samples, sampleCount, left, right, top, bottom, xPoints, yPoints, maxSpeed);

    // Draw horizontal grid lines
    for (int line = 0; line <= 2; line++) {
        int y = top + (graphHeight * line) / 2;
        tft.drawLine(left, y, right, y, TFT_DARKGREY);
    }

    int lastX = -1;
    int lastY = -1;
    for (uint8_t i = 0; i < sampleCount; i++) {
        int x = xPoints[i];
        int y = yPoints[i];
        if (lastX >= 0) {
            tft.drawLine(lastX, lastY, x, y, TFT_GREEN);
        }
        tft.fillCircle(x, y, 2, TFT_GREEN);
        lastX = x;
        lastY = y;
    }

    // Axis labels
    tft.drawString("0", left, bottom + 2, 2);
    tft.drawString(String((int)maxSpeed), left, top - 8, 2);
    tft.drawString("sec", right - 30, bottom + 2, 2);
}

DisplayMode getCurrentMode()
{
    return currentMode;
}