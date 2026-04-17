# Code Documentation

This document describes the structure and functionality of the bicycle computer project. The system consists of several modules, each responsible for specific tasks.

---

## Module Overview

| File | Description |
|------|-------------|
| `main.cpp` | Main loop and program coordination |
| `startup.cpp` / `startup.h` | Display initialization and startup animation |
| `interrupts.cpp` / `interrupts.h` | Interrupt handling and button processing |
| `metrics.cpp` / `metrics.h` | Speed and distance calculation |
| `display.cpp` / `display.h` | UI display and screen management |
| `graph.cpp` / `graph.h` | Graph calculation and data preparation |

---

## 1. main.cpp

The `main.cpp` is the core of the program and coordinates all other modules.

### Structure

```cpp
TFT_eSPI tft = TFT_eSPI();
int impulseTimeout = 0;
```

**Global Variables:**
- `tft` – TFT display object for all display operations
- `impulseTimeout` – Counter for speed timeout (no impulses)

### setup()

The `setup()` function initializes the system:

1. **Display initialization** – `setupDisplay(tft)` configures the TFT display correctly
2. **Startup screen** – `showStartupScreen(tft)` shows the startup animation
3. **Interrupt configuration** – `setupInterrupts()` enables the button interrupts

### loop()

The main loop runs every second and performs the following tasks:

**Speed timeout:**
```cpp
if (millis() - lastDistanceUpdateTime > 1000) {
    impulseTimeout = impulseTimeout + 1;
    if (impulseTimeout > 5) {
        speed = 0;  // No impulses for 5 seconds → speed = 0
    } else {
        speed = speed * 0.8;  // Gradually reduce speed
    }
}
```

- If no magnet impulses are received for more than 5 seconds, speed is set to 0
- Otherwise, speed is continuously reduced (braking)

**Display update:**
- `displayCurrentMode()` – Displays the current screen mode
- `updateUptime()` – Updates the operating time (hours:minutes:seconds)

**Sleep mode:**
```cpp
if (sleepRequested) {
    // Wait for button to be released
    tft.fillScreen(TFT_BLACK);
    while (digitalRead(KNOPF1_PIN) == HIGH) {
        delay(10);
    }
    // Enable light sleep and wake on button interrupt
    esp_sleep_enable_ext1_wakeup(1ULL << KNOPF1_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_deep_sleep_start();
    sleepRequested = false;
}
```

- When sleep is requested, the program waits for button release
- The ESP32 is put into deep sleep and woken up by Button 1

---

## 2. startup.cpp / startup.h

This module handles display initialization and shows a startup animation.

### startup.h

```cpp
void setupDisplay(TFT_eSPI& tft);
void showStartupScreen(TFT_eSPI& tft);
```

### setupDisplay()

Initializes the TFT display:
```cpp
tft.init();           // Initialize display
tft.setRotation(1);   // Landscape mode
tft.fillScreen(TFT_BLACK);  // Black background
tft.setTextColor(TFT_WHITE);  // White text
tft.setTextSize(1);   // Default text size
```

### showStartupScreen()

Displays the startup screen with a progress bar animation:
- Title "Fahrradcomputer" is displayed centered
- A progress bar fills over 5 seconds (50 steps at 100ms each)
- After completion, the display mode is set to `MAIN_DISPLAY`

---

## 3. interrupts.cpp / interrupts.h

This module contains all Interrupt Service Routines (ISRs) and button logic.

### Pin Definitions (interrupts.h)

```cpp
#define KNOPF1_PIN 35   // Button 1 (GPIO 35) – Menu / Confirmation
#define KNOPF2_PIN 0    // Button 2 (GPIO 0) – Mode Change / Sleep
#define MAGNET_PIN 33   // Magnet sensor (GPIO 33) – Speed impulses
```

### Button Modes

**button1Mode (Button 1):**
| Mode | Description |
|------|-------------|
| `DISPLAY_MODE_SELECTOR` | Switches between main display, graph and menu |
| `MENU_NAVIGATOR` | Navigates the menu (currently only graph options) |
| `GRAPH_TIME_SELECTOR` | Selection of graph time span (30s/60s/120s) |

**button2Mode (Button 2):**
| Mode | Description |
|------|-------------|
| `SLEEP_REQUESTER` | Requests sleep mode |
| `MENU_SELECTOR` | Confirms menu selection |

### setupInterrupts()

Configures the pins and enables interrupts:
```cpp
pinMode(KNOPF1_PIN, INPUT);
pinMode(KNOPF2_PIN, INPUT);
pinMode(MAGNET_PIN, INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(KNOPF1_PIN), isrKnopf1, RISING);
attachInterrupt(digitalPinToInterrupt(KNOPF2_PIN), isrKnopf2, RISING);
attachInterrupt(digitalPinToInterrupt(MAGNET_PIN), isrMagnet, RISING);
```

**Pin assignment:**
- `KNOPF1_PIN` (GPIO 35) → `isrKnopf1` – Button 1 (Menu/Confirmation)
- `KNOPF2_PIN` (GPIO 0) → `isrKnopf2` – Button 2 (Mode Change/Sleep)
- `MAGNET_PIN` (GPIO 33) → `isrMagnet` – Magnet sensor

### isrKnopf1()

Interrupt handler for Button 1 (GPIO 35):
- **Debounce:** 30ms delay to prevent button bounce
- **DISPLAY_MODE_SELECTOR:** Changes the display mode
- **GRAPH_TIME_SELECTOR:** Cycles through 30s → 60s → 120s → 30s

### isrKnopf2()

Interrupt handler for Button 2 (GPIO 0):
- Checks if menu mode is active → switches to `MENU_SELECTOR`
- Otherwise → `SLEEP_REQUESTER` (request sleep)
- In menu: Transition between modes or confirmation of graph selection

### isrMagnet()

Interrupt handler for magnet sensor:
- Updates `lastDistanceUpdateTime` for timeout tracking
- Calls `tagkm()` → increases kilometer count
- Calls `calculateSpeed()` → calculates speed
- Updates display in main mode

### handleButton1ModeTransition()

Switches the Button 1 mode:
```
DISPLAY_MODE_SELECTOR → MENU_NAVIGATOR → GRAPH_TIME_SELECTOR
```

### handleGraphOptionSelection()

Cyclic selection of graph time span:
```
OPTION_30S → OPTION_60S → OPTION_120S → OPTION_30S
```

### handleGraphOptionSelectionConfimation()

Confirms the graph selection and sets the time span:
- 30s → `setGraphTimeframeSeconds(30)`
- 60s → `setGraphTimeframeSeconds(60)`
- 120s → `setGraphTimeframeSeconds(120)`
- Returns to normal mode

---

## 4. metrics.cpp / metrics.h

This module calculates all bicycle-related data: speed, distance, time and graph history.

### Global Variables (metrics.h)

```cpp
extern volatile double km;           // Distance traveled in km
extern volatile double speed;        // Current speed in km/h
extern const double weg;             // Distance per pulse (2.15 m)
extern volatile unsigned long pulseIntervalMs;  // Time between pulses
extern volatile unsigned long lastPulseTime;    // Timestamp of last pulse
extern volatile int lastSpeedUpdateTime;        // Last speed update
extern volatile int uptimeSeconds;   // Operating time: seconds
extern volatile int uptimeMinutes;   // Operating time: minutes
extern volatile int uptimeHours;     // Operating time: hours
```

### tagkm()

Increases the kilometer count by one pulse:
```cpp
void tagkm() {
    km += wegInKm;  // 0.00215 km (2.15 m in km)
}
```

### calculateSpeed()

Calculates speed based on pulse interval:
```cpp
void calculateSpeed() {
    unsigned long currentTime = millis();
    pulseIntervalMs = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
    // Speed = (distance / time) * 3.6 (m/s → km/h)
    speed = (weg / (pulseIntervalMs / 1000.0)) * 3.6;
}
```

### getClampedSpeed()

Safety function that sets negative speeds to 0:
```cpp
double getClampedSpeed(double value) {
    return value < 0 ? 0 : value;
}
```

### Graph History

The system stores the last 120 speed values for graph display:

```cpp
static const uint8_t GRAPH_HISTORY_MAX = 120;
static double speedHistory[GRAPH_HISTORY_MAX];
static uint8_t speedHistoryCount = 0;
static uint8_t speedHistoryPos = 0;
static uint8_t graphTimeframeSeconds = 30;
```

**Functions:**

| Function | Description |
|----------|-------------|
| `sampleSpeedHistory()` | Stores current speed in ring buffer |
| `setGraphTimeframeSeconds(uint8_t)` | Sets display time span (30/60/120 seconds) |
| `getGraphTimeframeSeconds()` | Returns current time span |
| `getSpeedGraphData(double*, uint8_t&)` | Copies historical data to target array |

### updateUptime()

Updates the operating time every second:
```cpp
void updateUptime() {
    if (currentTime - lastUpdateTime >= 1000) {
        uptimeSeconds++;
        if (uptimeSeconds >= 60) {
            uptimeSeconds = 0;
            uptimeMinutes++;
        }
        if (uptimeMinutes >= 60) {
            uptimeMinutes = 0;
            uptimeHours++;
        }
        sampleSpeedHistory();  // Store speed value
    }
}
```

---

## 5. display.cpp / display.h

This module manages the screen display and various display modes.

### Display Modes (display.h)

```cpp
enum DisplayMode {
    STARTUP,      // Startup animation
    MAIN_DISPLAY, // Main display (speed, km, time)
    GRAPH,        // Speed history
    MENU          // Settings menu
};
```

### updateDisplayMode()

Cycles through the modes:
```
STARTUP → MAIN_DISPLAY → GRAPH → MENU → MAIN_DISPLAY
```

### displayCurrentMode()

Displays the current mode – calls the appropriate display function.

### updateMainDisplay()

Shows the main display with the following layout:

```
┌─────────────────┬─────────────────┐
│                 │                 │
│    25.50        │    12.345       │
│    km/h         │       km        │
├─────────────────┴─────────────────┤
│           01:23:45                │
└───────────────────────────────────┘
```

- **Left side:** Current speed in km/h (large)
- **Right side:** Total kilometers (large)
- **Bottom:** Operating time (hours:minutes:seconds)

### showMenu()

Displays the menu with the following options:

**Top level menu:**
- "Menu" (title)
- "> Graph resolution" (selectable)

**Graph resolution submenu:**
- "> 30s" / " 30s"
- "> 60s" / " 60s"
- "> 120s" / " 120s"
- "Press Button 2 to save"

The arrow `>` indicates the current selection.

### showGraph()

Displays the speed history:

- **Title:** "Speed history [30s|60s|120s]"
- **Graph area:** Rectangle with grid lines
- **Data points:** Green line with dots
- **Axis labels:**
  - Left: 0 (bottom) and max speed (top)
  - Bottom: "sec" (time axis)

**Empty state:** "No speed data yet" when no data available

### getCurrentMode()

Returns the current display mode (for other modules).

---

## 6. graph.cpp / graph.h

This module calculates the coordinates for graph display.

### buildGraphPoints()

Converts speed data to screen coordinates:

```cpp
void buildGraphPoints(const double* samples, uint8_t sampleCount, 
                      int left, int right, int top, int bottom,
                      int* xPoints, int* yPoints, double& maxSpeed)
```

**Algorithm:**
1. **Determine max speed:** Highest value + 2 km/h buffer (minimum: 10 km/h)
2. **X coordinates:** Evenly distributed across graph width
3. **Y coordinates:** Normalized to graph height (0 = bottom, maxSpeed = top)

**Calculation:**
```cpp
xPoints[i] = left + (graphWidth * i) / (sampleCount - 1);
yPoints[i] = bottom - (samples[i] / maxSpeed) * graphHeight;
```

### getGraphMaxSpeed()

Calculates the maximum speed in the data set:
```cpp
double getGraphMaxSpeed(const double* samples, uint8_t sampleCount) {
    double maxSpeed = 10;
    for (uint8_t i = 0; i < sampleCount; i++) {
        if (samples[i] > maxSpeed) {
            maxSpeed = samples[i];
        }
    }
    maxSpeed += 2;  // Buffer for better display
    if (maxSpeed < 10) maxSpeed = 10;
    return maxSpeed;
}
```

---

## Program Flow

### Startup Sequence

1. `setup()` is executed once
2. Display is initialized (`setupDisplay`)
3. Startup animation shows progress bar (`showStartupScreen`)
4. Interrupts are enabled (`setupInterrupts`)
5. Mode switches to `MAIN_DISPLAY`

### Main Loop (loop)

```
┌─────────────────────────────────────┐
│ 1. Check speed timeout              │
│    - No impulses > 5s → speed = 0   │
│    - Otherwise: speed decays        │
├─────────────────────────────────────┤
│ 2. Update display                   │
│    - Depending on mode:             │
│      • MAIN_DISPLAY: Speed/km/Uptime│
│      • GRAPH: Speed graph           │
│      • MENU: Settings menu          │
├─────────────────────────────────────┤
│ 3. Update uptime                    │
│    - Every second: time + graph data│
├─────────────────────────────────────┤
│ 4. Check sleep mode                 │
│    - If requested: Deep Sleep       │
└─────────────────────────────────────┘
         ↓ 1 second delay ↓
```

### Interrupt Handling

**Magnet impulse (each wheel rotation):**
```
isrMagnet() → tagkm() + calculateSpeed() → displayCurrentMode()
```

**Button 1 (physical):**
```
isrKnopf1() →
  DISPLAY_MODE_SELECTOR: updateDisplayMode()
  GRAPH_TIME_SELECTOR: handleGraphOptionSelection()
```

**Button 2 (physical):**
```
isrKnopf2() → 
  If MENU: MENU_SELECTOR → handleButton1ModeTransition()
  Otherwise: SLEEP_REQUESTER → sleepRequested = true
```

---

## Technical Details

### Hardware Used

- **ESP32**
- **TFT Display**
- **Magnet sensor** (Reed contact or Hall sensor)
- **2 Buttons** for user interaction

### Pin Assignment

| GPIO | Function |
|------|----------|
| 0 | Button 2 (Mode Change/Sleep) |
| 33 | Magnet sensor |
| 35 | Button 1 (Menu/Confirmation) |

### Important Constants

```cpp
weg = 2.15 m           // Distance per wheel rotation
DEBOUNCE_DELAY = 30ms  // Debounce time for buttons
GRAPH_HISTORY_MAX = 120 // Maximum storage slots
```

---

## Module Dependencies

```
main.cpp
├── startup.h → startup.cpp
├── interrupts.h → interrupts.cpp
├── display.h → display.cpp
└── metrics.h → metrics.cpp

display.cpp
├── metrics.h
├── startup.h
└── graph.h

interrupts.cpp
├── interrupts.h
├── metrics.h
├── display.h
└── graph.h

graph.cpp
└── graph.h
```