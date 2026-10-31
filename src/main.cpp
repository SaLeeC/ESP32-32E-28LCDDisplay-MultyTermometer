#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TFT_eSPI.h>
#include <math.h>

constexpr uint8_t SENSOR_PIN = 27;
constexpr uint8_t BUTTON_PIN = 0;
constexpr uint8_t MAX_PROBES = 8;
constexpr uint16_t HISTORY_SIZE = 120;
constexpr uint32_t SAMPLE_INTERVAL_MS = 1000;
constexpr uint32_t CONVERSION_MS = 750;
constexpr uint32_t DEBOUNCE_MS = 40;

struct Probe {
    DeviceAddress address;
    float temperature;
    float history[HISTORY_SIZE];
};

OneWire bus(SENSOR_PIN);
DallasTemperature sensors(&bus);
TFT_eSPI display;
Probe probes[MAX_PROBES];
const uint16_t colors[MAX_PROBES] = {
    TFT_RED, TFT_GREEN, TFT_CYAN, TFT_YELLOW,
    TFT_MAGENTA, TFT_ORANGE, TFT_WHITE, TFT_PINK
};
uint8_t probeCount = 0;
uint16_t historyHead = 0;
uint16_t historyCount = 0;
uint32_t conversionStarted = 0;
bool conversionPending = false;
bool graphView = false;
bool redraw = true;

void drawValues() {
    display.setTextFont(2);
    for (uint8_t i = 0; i < probeCount; ++i) {
        char text[48];
        if (isfinite(probes[i].temperature)) {
            snprintf(text, sizeof(text), "S%u: %.2f C", i + 1,
                     probes[i].temperature);
        } else {
            snprintf(text, sizeof(text), "S%u: -- (non disponibile)", i + 1);
        }
        display.setTextColor(colors[i], TFT_BLACK);
        display.drawString(text, 12, 36 + i * 22);
    }
}

void drawGraph() {
    constexpr int LEFT = 42;
    constexpr int RIGHT = 310;
    constexpr int TOP = 42;
    constexpr int BOTTOM = 184;
    float low = INFINITY;
    float high = -INFINITY;
    for (uint8_t p = 0; p < probeCount; ++p) {
        for (uint16_t i = 0; i < historyCount; ++i) {
            float value = probes[p].history[i];
            if (isfinite(value)) {
                low = fminf(low, value);
                high = fmaxf(high, value);
            }
        }
    }
    display.setTextFont(1);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.drawFastVLine(LEFT, TOP, BOTTOM - TOP + 1, TFT_DARKGREY);
    display.drawFastHLine(LEFT, BOTTOM, RIGHT - LEFT + 1, TFT_DARKGREY);
    display.drawString("-119 s", LEFT, BOTTOM + 3);
    display.drawRightString("ora", RIGHT, BOTTOM + 3);
    if (isfinite(low)) {
        low = floorf(low - 1.0f);
        high = ceilf(high + 1.0f);
        char label[16];
        snprintf(label, sizeof(label), "%.0f C", high);
        display.drawString(label, 0, TOP);
        snprintf(label, sizeof(label), "%.0f C", low);
        display.drawString(label, 0, BOTTOM - 8);
        for (uint8_t p = 0; p < probeCount; ++p) {
            bool previousValid = false;
            int previousX = 0;
            int previousY = 0;
            for (uint16_t i = 0; i < historyCount; ++i) {
                uint16_t index =
                    (historyHead + HISTORY_SIZE - historyCount + i) % HISTORY_SIZE;
                float value = probes[p].history[index];
                if (!isfinite(value)) {
                    previousValid = false;
                    continue;
                }
                int x = RIGHT - (historyCount - 1 - i) *
                                (RIGHT - LEFT) / (HISTORY_SIZE - 1);
                int y = BOTTOM - lroundf((value - low) * (BOTTOM - TOP) /
                                        (high - low));
                if (previousValid) {
                    display.drawLine(previousX, previousY, x, y, colors[p]);
                } else {
                    display.drawPixel(x, y, colors[p]);
                }
                previousX = x;
                previousY = y;
                previousValid = true;
            }
        }
    } else {
        display.drawString("In attesa di misure valide", LEFT + 8, 100);
    }
    for (uint8_t p = 0; p < probeCount; ++p) {
        char label[20];
        if (isfinite(probes[p].temperature)) {
            snprintf(label, sizeof(label), "S%u %.1f", p + 1,
                     probes[p].temperature);
        } else {
            snprintf(label, sizeof(label), "S%u --", p + 1);
        }
        display.setTextColor(colors[p], TFT_BLACK);
        display.drawString(label, (p % 4) * 80, 200 + (p / 4) * 10);
    }
}

void drawScreen() {
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextFont(2);
    display.drawString(graphView ? "Andamento temperature" : "Temperature sonde",
                       8, 8);
    if (probeCount == 0) {
        display.drawString("Nessuna sonda DS18B20", 8, 60);
        display.drawString("Verificare cavi e riavviare", 8, 84);
    } else if (graphView) {
        drawGraph();
    } else {
        drawValues();
    }
    display.setTextFont(1);
    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.drawString("BOOT: valori / grafico", 8, 230);
    redraw = false;
}

void pollButton(uint32_t now) {
    static bool lastRaw = HIGH;
    static bool stable = HIGH;
    static uint32_t changedAt = 0;
    bool raw = digitalRead(BUTTON_PIN);
    if (raw != lastRaw) {
        lastRaw = raw;
        changedAt = now;
    }
    if (raw != stable && uint32_t(now - changedAt) >= DEBOUNCE_MS) {
        stable = raw;
        if (stable == LOW) {
            graphView = !graphView;
            redraw = true;
        }
    }
}

void sampleTemperatures(uint32_t now) {
    if (probeCount == 0) {
        return;
    }
    if (conversionPending &&
        uint32_t(now - conversionStarted) >= CONVERSION_MS) {
        for (uint8_t i = 0; i < probeCount; ++i) {
            float value = sensors.getTempC(probes[i].address);
            if (!isfinite(value) || value == DEVICE_DISCONNECTED_C ||
                value < -55.0f || value > 125.0f) {
                value = NAN;
            }
            probes[i].temperature = value;
            probes[i].history[historyHead] = value;
        }
        historyHead = (historyHead + 1) % HISTORY_SIZE;
        if (historyCount < HISTORY_SIZE) {
            ++historyCount;
        }
        conversionPending = false;
        redraw = true;
    }
    if (!conversionPending &&
        uint32_t(now - conversionStarted) >= SAMPLE_INTERVAL_MS) {
        sensors.requestTemperatures();
        conversionStarted = now;
        conversionPending = true;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    display.init();
    display.setRotation(1);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    sensors.begin();
    DeviceAddress address;
    uint8_t found = 0;
    for (uint8_t i = 0; i < sensors.getDeviceCount(); ++i) {
        if (!sensors.getAddress(address, i) || address[0] != 0x28) {
            continue;
        }
        ++found;
        if (probeCount == MAX_PROBES) {
            continue;
        }
        Probe &probe = probes[probeCount];
        memcpy(probe.address, address, sizeof(DeviceAddress));
        probe.temperature = NAN;
        for (float &value : probe.history) {
            value = NAN;
        }
        Serial.printf("S%u ROM:", probeCount + 1);
        for (uint8_t byte : probe.address) {
            Serial.printf(" %02X", byte);
        }
        Serial.println();
        ++probeCount;
    }
    if (found > MAX_PROBES) {
        Serial.printf("Limite %u sonde: %u sonde ignorate\n",
                      MAX_PROBES, found - MAX_PROBES);
    }
    sensors.setResolution(12);
    sensors.setWaitForConversion(false);
    if (probeCount > 0) {
        sensors.requestTemperatures();
        conversionStarted = millis();
        conversionPending = true;
    }
    drawScreen();
}

void loop() {
    uint32_t now = millis();
    pollButton(now);
    sampleTemperatures(now);
    if (redraw) {
        drawScreen();
    }
}
