#include "interrupts.h"
#include "metrics.h"  
#include "display.h"  
#include "graph.h"

#define DEBOUNCE_DELAY 30 
volatile unsigned long lastDistanceUpdateTime = 0;
button1Mode Button1Mode = DISPLAY_MODE_SELECTOR;
button1Mode NextButton1Mode = GRAPH_TIME_SELECTOR;
button2Mode Button2Mode = SLEEP_REQUESTER;
GraphOptions GraphOptionSelection = OPTION_30S;
bool graphResolutionSubmenuActive = false;
void handleButton1ModeTransition();
void handleGraphOptionSelection();
void handleGraphOptionSelectionConfimation();


bool sleepRequested = false;


void setupInterrupts() {
    pinMode(KNOPF1_PIN, INPUT);
    pinMode(KNOPF2_PIN, INPUT);
    pinMode(MAGNET_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(KNOPF1_PIN), isrKnopf2, RISING);
    attachInterrupt(digitalPinToInterrupt(KNOPF2_PIN), isrKnopf1, RISING);
    attachInterrupt(digitalPinToInterrupt(MAGNET_PIN), isrMagnet, RISING);
}

void isrKnopf1() {
    static unsigned long lastPressTime1 = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastPressTime1 > DEBOUNCE_DELAY) {
        switch (Button1Mode) {
            case DISPLAY_MODE_SELECTOR:
                updateDisplayMode();
                displayCurrentMode();
                break;
            case MENU_NAVIGATOR:
                // Top-level menu navigation (only Graph resolution entry for now).
                break;
            case GRAPH_TIME_SELECTOR:
                handleGraphOptionSelection();
                break;
        }
        lastPressTime1 = currentTime;
    }
}

void isrKnopf2() {
    static unsigned long lastPressTime2 = 0;
    unsigned long currentTime = millis();

    
    if (currentTime - lastPressTime2 > DEBOUNCE_DELAY) {
        if (getCurrentMode() == MENU) {
        Button2Mode = MENU_SELECTOR;
        } else {
        Button2Mode = SLEEP_REQUESTER;
        }
        switch (Button2Mode)
        {
        case SLEEP_REQUESTER:
            // Request sleep
            sleepRequested = true;
            lastPressTime2 = currentTime;
            break;
        case MENU_SELECTOR:
            if (Button1Mode != GRAPH_TIME_SELECTOR) {
                handleButton1ModeTransition();
            } else if (Button1Mode == GRAPH_TIME_SELECTOR) {
                handleGraphOptionSelectionConfimation();
            }
            break;
        }
    }   
}

void isrMagnet() {
    static unsigned long lastPressTimeM = 0;
    unsigned long currentTime = millis();
    lastDistanceUpdateTime = currentTime;

    // Handle magnet pulse - increment km and update speed
    if (currentTime - lastPressTimeM > DEBOUNCE_DELAY) {
        tagkm();
        calculateSpeed();
        if (getCurrentMode() == MAIN_DISPLAY) {
            displayCurrentMode();
        }
        lastPressTimeM = currentTime;
    }
    
}

void handleButton1ModeTransition() {
    switch (Button1Mode) {
        case DISPLAY_MODE_SELECTOR:
            Button1Mode = MENU_NAVIGATOR;
            break;
        case MENU_NAVIGATOR:
            Button1Mode = GRAPH_TIME_SELECTOR;
            graphResolutionSubmenuActive = true;
            break;
        case GRAPH_TIME_SELECTOR:
            break;
    }
    displayCurrentMode();
}

void handleGraphOptionSelection() {
    switch (GraphOptionSelection)
    {
    case OPTION_30S:
        GraphOptionSelection = OPTION_60S;
        break;
    case OPTION_60S:
        GraphOptionSelection = OPTION_120S;
        break;
    case OPTION_120S:
        GraphOptionSelection = OPTION_30S;
        break;
    default:
        GraphOptionSelection = OPTION_30S;
        break;
    }
    displayCurrentMode();
}

void handleGraphOptionSelectionConfimation() {
    switch (GraphOptionSelection)
    {
    case OPTION_30S:
        setGraphTimeframeSeconds(30);
        break;
    case OPTION_60S:
        setGraphTimeframeSeconds(60);
        break;
    case OPTION_120S:
        setGraphTimeframeSeconds(120);
        break;
    default:
        setGraphTimeframeSeconds(30);
        break;
    }
    graphResolutionSubmenuActive = false;
    Button1Mode = DISPLAY_MODE_SELECTOR;
    Button2Mode = SLEEP_REQUESTER;
    displayCurrentMode();
}