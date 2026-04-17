#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <esp_sleep.h>
#include "startup.h"
#include "interrupts.h"
#include "display.h"
#include "metrics.h"

TFT_eSPI tft = TFT_eSPI();
int impulseTimeout = 0;

void setup() {
  setupDisplay(tft);
  showStartupScreen(tft);
  setupInterrupts();
  
}

void loop() {
  if (millis() - lastDistanceUpdateTime > 1000) {
    impulseTimeout = impulseTimeout + 1;
    if (impulseTimeout > 5) {
      speed = 0;
    } else {
      speed = speed * 0.8;
    }

  }
  displayCurrentMode();
  updateUptime();
  if (sleepRequested) {
    // Wait for button to be released
    tft.fillScreen(TFT_BLACK);
    while (digitalRead(KNOPF1_PIN) == HIGH) {
      delay(10);
    }
    // Enter light sleep and wake on button press
    esp_sleep_enable_ext1_wakeup(1ULL << KNOPF1_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_deep_sleep_start();
    sleepRequested = false;
  }
  delay(1000); 
}

