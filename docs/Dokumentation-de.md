# Code-Dokumentation

Dieses Dokument beschreibt die Struktur und Funktionsweise des Fahrradcomputer-Projekts. Das System besteht aus mehreren Modulen, die jeweils für spezifische Aufgaben verantwortlich sind.

---

## Übersicht der Module

| Datei | Beschreibung |
|-------|---------------|
| `main.cpp` | Hauptschleife und Programmkoordination |
| `startup.cpp` / `startup.h` | Display-Initialisierung und Startanimation |
| `interrupts.cpp` / `interrupts.h` | Interrupt-Handling und Button-Verarbeitung |
| `metrics.cpp` / `metrics.h` | Geschwindigkeits- und Streckenberechnung |
| `display.cpp` / `display.h` | UI-Darstellung und Bildschirmverwaltung |
| `graph.cpp` / `graph.h` | Graph-Berechnung und Datenaufbereitung |

---

## 1. main.cpp

Die `main.cpp` bildet das Herzstück des Programms und koordiniert alle anderen Module.

### Aufbau

```cpp
TFT_eSPI tft = TFT_eSPI();
int impulseTimeout = 0;
```

**Globale Variablen:**
- `tft` – TFT-Display-Objekt für alle Display-Operationen
- `impulseTimeout` – Zähler für Geschwindigkeits-Timeout (keine Impulse mehr)

### setup()

Die `setup()`-Funktion initialisiert das System:

1. **Display-Initialisierung** – `setupDisplay(tft)` richtet das TFT-Display korrekt ein
2. **Startbildschirm** – `showStartupScreen(tft)` zeigt die Startup-Animation
3. **Interrupt-Konfiguration** – `setupInterrupts()` aktiviert die Button-Interrupts

### loop()

Die Hauptschleife wird jede Sekunde ausgeführt und erledigt folgende Aufgaben:

**Geschwindigkeits-Timeout:**
```cpp
if (millis() - lastDistanceUpdateTime > 1000) {
    impulseTimeout = impulseTimeout + 1;
    if (impulseTimeout > 5) {
        speed = 0;  // Keine Impulse seit 5 Sekunden → Geschwindigkeit auf 0
    } else {
        speed = speed * 0.8;  // Geschwindigkeit langsam abfallen lassen
    }
}
```

- Wenn länger als 5 Sekunden keine Magnet-Impulse empfangen werden, wird die Geschwindigkeit auf 0 gesetzt
- Andernfalls wird die Geschwindigkeit kontinuierlich reduziert (Abbremsen)

**Display-Aktualisierung:**
- `displayCurrentMode()` – Zeigt den aktuellen Bildschirmmodus an
- `updateUptime()` – Aktualisiert die Betriebszeit (Stunden:Minuten:Sekunden)

**Schlafmodus:**
```cpp
if (sleepRequested) {
    // Warten bis Button losgelassen wird
    tft.fillScreen(TFT_BLACK);
    while (digitalRead(KNOPF1_PIN) == HIGH) {
        delay(10);
    }
    // Light Sleep aktivieren und auf Button-Interrupt aufwachen
    esp_sleep_enable_ext1_wakeup(1ULL << KNOPF1_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
    esp_deep_sleep_start();
    sleepRequested = false;
}
```

- Wenn Schlaf angefordert wird, wartet das Programm auf Button-Loslassen
- Der ESP32 wird in Deep Sleep versetzt und durch Button 1 aufgeweckt

---

## 2. startup.cpp / startup.h

Dieses Modul übernimmt die Display-Initialisierung und zeigt eine Startanimation.

### startup.h

```cpp
void setupDisplay(TFT_eSPI& tft);
void showStartupScreen(TFT_eSPI& tft);
```

### setupDisplay()

Initialisiert das TFT-Display:
```cpp
tft.init();           // Display initialisieren
tft.setRotation(1);   // Querformat (Landscape)
tft.fillScreen(TFT_BLACK);  // Schwarzer Hintergrund
tft.setTextColor(TFT_WHITE);  // Weißer Text
tft.setTextSize(1);   // Standard-Textgröße
```

### showStartupScreen()

Zeigt den Startbildschirm mit einer Ladebalken-Animation:
- Titel "Fahrradcomputer" wird zentriert angezeigt
- Ein Ladebalken füllt sich über 5 Sekunden (50 Schritte à 100ms)
- Nach Abschluss wird der Display-Modus auf `MAIN_DISPLAY` gesetzt

---

## 3. interrupts.cpp / interrupts.h

Dieses Modul enthält alle Interrupt Service Routines (ISRs) und die Button-Logik.

### Pin-Definitionen (interrupts.h)

```cpp
#define KNOPF1_PIN 35   // Button 1 (GPIO 35) – Menü / Bestätigung
#define KNOPF2_PIN 0    // Button 2 (GPIO 0) – Modus-Wechsel / Schlaf
#define MAGNET_PIN 33   // Magnet-Sensor (GPIO 33) – Geschwindigkeitsimpulse
```

### Button-Modi

**button1Mode (Button 1):**
| Modus | Beschreibung |
|-------|---------------|
| `DISPLAY_MODE_SELECTOR` | Wechselt zwischen Hauptanzeige, Graph und Menü |
| `MENU_NAVIGATOR` | Navigiert im Menü (aktuell nur Graph-Optionen) |
| `GRAPH_TIME_SELECTOR` | Auswahl der Graph-Zeitspanne (30s/60s/120s) |

**button2Mode (Button 2):**
| Modus | Beschreibung |
|-------|---------------|
| `SLEEP_REQUESTER` | Fordert Schlafmodus an |
| `MENU_SELECTOR` | Bestätigt Menü-Auswahl |

### setupInterrupts()

Konfiguriert die Pins und aktiviert Interrupts:
```cpp
pinMode(KNOPF1_PIN, INPUT);
pinMode(KNOPF2_PIN, INPUT);
pinMode(MAGNET_PIN, INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(KNOPF1_PIN), isrKnopf1, RISING);
attachInterrupt(digitalPinToInterrupt(KNOPF2_PIN), isrKnopf2, RISING);
attachInterrupt(digitalPinToInterrupt(MAGNET_PIN), isrMagnet, RISING);
```

**Pin-Zuordnung:**
- `KNOPF1_PIN` (GPIO 35) → `isrKnopf1` – Button 1 (Menü/Bestätigung)
- `KNOPF2_PIN` (GPIO 0) → `isrKnopf2` – Button 2 (Modus-Wechsel/Schlaf)
- `MAGNET_PIN` (GPIO 33) → `isrMagnet` – Magnet-Sensor

### isrKnopf1()

Interrupt-Handler für Button 1 (GPIO 35):
- **Debounce:** 30ms Verzögerung gegen Tasten-Flatter
- **DISPLAY_MODE_SELECTOR:** Wechselt den Anzeigemodus
- **GRAPH_TIME_SELECTOR:** Zyklisch durch 30s → 60s → 120s → 30s

### isrKnopf2()

Interrupt-Handler für Button 2 (GPIO 0):
- Prüft ob Menü-Modus aktiv → wechselt zu `MENU_SELECTOR`
- Andernfalls → `SLEEP_REQUESTER` (Schlaf anfordern)
- Im Menü: Übergang zwischen Modi oder Bestätigung der Graph-Auswahl

### isrMagnet()

Interrupt-Handler für Magnet-Sensor:
- Aktualisiert `lastDistanceUpdateTime` für Timeout-Tracking
- Ruft `tagkm()` auf → erhöht Kilometerstand
- Ruft `calculateSpeed()` auf → berechnet Geschwindigkeit
- Aktualisiert Display im Hauptmodus

### handleButton1ModeTransition()

Wechselt den Button 1-Modus:
```
DISPLAY_MODE_SELECTOR → MENU_NAVIGATOR → GRAPH_TIME_SELECTOR
```

### handleGraphOptionSelection()

Zyklische Auswahl der Graph-Zeitspanne:
```
OPTION_30S → OPTION_60S → OPTION_120S → OPTION_30S
```

### handleGraphOptionSelectionConfimation()

Bestätigt die Graph-Auswahl und setzt den Zeitraum:
- 30s → `setGraphTimeframeSeconds(30)`
- 60s → `setGraphTimeframeSeconds(60)`
- 120s → `setGraphTimeframeSeconds(120)`
- Zurück zum normalen Modus

---

## 4. metrics.cpp / metrics.h

Dieses Modul berechnet alle fahrradbezogenen Daten: Geschwindigkeit, Strecke, Zeit und Graph-Historie.

### Globale Variablen (metrics.h)

```cpp
extern volatile double km;           // Zurückgelegte Strecke in km
extern volatile double speed;        // Aktuelle Geschwindigkeit in km/h
extern const double weg;             // Strecke pro Impuls (2.15 m)
extern volatile unsigned long pulseIntervalMs;  // Zeit zwischen Impulsen
extern volatile unsigned long lastPulseTime;    // Zeitstempel letzter Impuls
extern volatile int lastSpeedUpdateTime;        // Letzte Geschwindigkeits-Aktualisierung
extern volatile int uptimeSeconds;   // Betriebszeit: Sekunden
extern volatile int uptimeMinutes;   // Betriebszeit: Minuten
extern volatile int uptimeHours;     // Betriebszeit: Stunden
```

### tagkm()

Erhöht den Kilometerstand um einen Impuls:
```cpp
void tagkm() {
    km += wegInKm;  // 0.00215 km (2.15 m in km)
}
```

### calculateSpeed()

Berechnet die Geschwindigkeit basierend auf dem Impuls-Intervall:
```cpp
void calculateSpeed() {
    unsigned long currentTime = millis();
    pulseIntervalMs = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
    // Geschwindigkeit = (Strecke / Zeit) * 3.6 (m/s → km/h)
    speed = (weg / (pulseIntervalMs / 1000.0)) * 3.6;
}
```

### getClampedSpeed()

Sicherheitsfunktion, die negative Geschwindigkeiten auf 0 setzt:
```cpp
double getClampedSpeed(double value) {
    return value < 0 ? 0 : value;
}
```

### Graph-Historie

Das System speichert die letzten 120 Geschwindigkeitswerte für die Graph-Darstellung:

```cpp
static const uint8_t GRAPH_HISTORY_MAX = 120;
static double speedHistory[GRAPH_HISTORY_MAX];
static uint8_t speedHistoryCount = 0;
static uint8_t speedHistoryPos = 0;
static uint8_t graphTimeframeSeconds = 30;
```

**Funktionen:**

| Funktion | Beschreibung |
|----------|---------------|
| `sampleSpeedHistory()` | Speichert aktuelle Geschwindigkeit im Ring-Puffer |
| `setGraphTimeframeSeconds(uint8_t)` | Setzt Anzeige-Zeitraum (30/60/120 Sekunden) |
| `getGraphTimeframeSeconds()` | Gibt aktuellen Zeitraum zurück |
| `getSpeedGraphData(double*, uint8_t&)` | Kopiert historische Daten in Ziel-Array |

### updateUptime()

Aktualisiert die Betriebszeit jede Sekunde:
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
        sampleSpeedHistory();  // Geschwindigkeitswert speichern
    }
}
```

---

## 5. display.cpp / display.h

Dieses Modul verwaltet die Bildschirmanzeige und die verschiedenen Display-Modi.

### Display-Modi (display.h)

```cpp
enum DisplayMode {
    STARTUP,      // Startanimation
    MAIN_DISPLAY, // Hauptanzeige (Geschwindigkeit, km, Zeit)
    GRAPH,        // Geschwindigkeitsverlauf
    MENU          // Einstellungsmenü
};
```

### updateDisplayMode()

Schaltet zyklisch durch die Modi:
```
STARTUP → MAIN_DISPLAY → GRAPH → MENU → MAIN_DISPLAY
```

### displayCurrentMode()

Zeigt den aktuellen Modus an – ruft die entsprechende Anzeigefunktion auf.

### updateMainDisplay()

Zeigt die Hauptanzeige mit folgendem Layout:

```
┌─────────────────┬─────────────────┐
│                 │                 │
│    25.50        │    12.345       │
│    km/h         │       km        │
├─────────────────┴─────────────────┤
│           01:23:45                │
└───────────────────────────────────┘
```

- **Linke Seite:** Aktuelle Geschwindigkeit in km/h (groß)
- **Rechte Seite:** Gesamtkilometer (groß)
- **Unten:** Betriebszeit (Stunden:Minuten:Sekunden)

### showMenu()

Zeigt das Menü mit folgenden Optionen:

**Oberstes Menü:**
- "Menu" (Titel)
- "> Graph resolution" (auswählbar)

**Graph-Resolution Untermenü:**
- "> 30s" / " 30s"
- "> 60s" / " 60s"
- "> 120s" / " 120s"
- "Press Button 2 to save"

Der Pfeil `>` zeigt die aktuelle Auswahl.

### showGraph()

Zeigt den Geschwindigkeitsverlauf:

- **Titel:** "Speed history [30s|60s|120s]"
- **Graph-Bereich:** Rechteck mit Gitterlinien
- **Datenpunkte:** Grüne Linie mit Punkten
- **Achsenbeschriftung:**
  - Links: 0 (unten) und Max-Geschwindigkeit (oben)
  - Unten: "sec" (Zeitachse)

**Leerzustand:** "No speed data yet" wenn keine Daten vorhanden

### getCurrentMode()

Gibt den aktuellen Display-Modus zurück (für andere Module).

---

## 6. graph.cpp / graph.h

Dieses Modul berechnet die Koordinaten für die Graph-Darstellung.

### buildGraphPoints()

Konvertiert Geschwindigkeitsdaten in Bildschirmkoordinaten:

```cpp
void buildGraphPoints(const double* samples, uint8_t sampleCount, 
                      int left, int right, int top, int bottom,
                      int* xPoints, int* yPoints, double& maxSpeed)
```

**Algorithmus:**
1. **Max-Geschwindigkeit ermitteln:** Höchster Wert + 2 km/h Puffer (Minimum: 10 km/h)
2. **X-Koordinaten:** Gleichmäßig verteilt über die Graph-Breite
3. **Y-Koordinaten:** Normalisiert auf Graph-Höhe (0 = unten, maxSpeed = oben)

**Berechnung:**
```cpp
xPoints[i] = left + (graphWidth * i) / (sampleCount - 1);
yPoints[i] = bottom - (samples[i] / maxSpeed) * graphHeight;
```

### getGraphMaxSpeed()

Berechnet die maximale Geschwindigkeit im Datensatz:
```cpp
double getGraphMaxSpeed(const double* samples, uint8_t sampleCount) {
    double maxSpeed = 10;
    for (uint8_t i = 0; i < sampleCount; i++) {
        if (samples[i] > maxSpeed) {
            maxSpeed = samples[i];
        }
    }
    maxSpeed += 2;  // Puffer für bessere Darstellung
    if (maxSpeed < 10) maxSpeed = 10;
    return maxSpeed;
}
```

---

## Programmablauf

### Startsequenz

1. `setup()` wird einmalig ausgeführt
2. Display wird initialisiert (`setupDisplay`)
3. Startup-Animation zeigt Ladebalken (`showStartupScreen`)
4. Interrupts werden aktiviert (`setupInterrupts`)
5. Modus wechselt zu `MAIN_DISPLAY`

### Hauptschleife (loop)

```
┌─────────────────────────────────────┐
│ 1. Geschwindigkeits-Timeout prüfen  │
│    - Keine Impulse > 5s → speed = 0 │
│    - Sonst: Geschwindigkeit abfallen│
├─────────────────────────────────────┤
│ 2. Display aktualisieren            │
│    - Je nach Modus:                 │
│      • MAIN_DISPLAY: Speed/km/Uptime│
│      • GRAPH: Geschwindigkeitsgraph │
│      • MENU: Einstellungsmenü       │
├─────────────────────────────────────┤
│ 3. Uptime aktualisieren             │
│    - Jede Sekunde: Zeit + Graph-Daten│
├─────────────────────────────────────┤
│ 4. Schlafmodus prüfen               │
│    - Wenn angefordert: Deep Sleep   │
└─────────────────────────────────────┘
         ↓ 1 Sekunde Pause ↓
```

### Interrupt-Handling

**Magnet-Impuls (jeder Radumdrehung):**
```
isrMagnet() → tagkm() + calculateSpeed() → displayCurrentMode()
```

**Button 1 (physikalisch):**
```
isrKnopf2() → 
  Wenn MENU: MENU_SELECTOR → handleButton1ModeTransition()
  Sonst: SLEEP_REQUESTER → sleepRequested = true
```

**Button 2 (physikalisch):**
```
isrKnopf1() →
  DISPLAY_MODE_SELECTOR: updateDisplayMode()
  GRAPH_TIME_SELECTOR: handleGraphOptionSelection()
```

---

## Technische Details

### Verwendete Hardware

- **ESP32** 
- **TFT-Display** 
- **Magnet-Sensor** (Reed-Kontakt oder Hall-Sensor)
- **2 Buttons** für Benutzerinteraktion

### Pin-Belegung

| GPIO | Funktion |
|------|----------|
| 0 | Button 2 (Modus-Wechsel/Schlaf) |
| 33 | Magnet-Sensor |
| 35 | Button 1 (Menü/Bestätigung) |

### Wichtige Konstanten

```cpp
weg = 2.15 m           // Strecke pro Radumdrehung
DEBOUNCE_DELAY = 30ms  // Entprellzeit für Buttons
GRAPH_HISTORY_MAX = 120 // Maximale Speicherplätze
```

---

## Modulabhängigkeiten

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