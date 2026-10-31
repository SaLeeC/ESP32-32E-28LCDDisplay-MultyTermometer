/*
 * Progetto: ESP32-32E-28LCDDisplay-MultyTermometer.
 * Scopo: misurare la temperatura di piu sonde e presentare i dati sul display.
 *
 * RISORSE DELLA SCHEDA DI RIFERIMENTO
 * - ESP32-WROOM-32E con Wi-Fi e Bluetooth disponibili; le funzioni wireless
 *   non sono utilizzate dalla versione attuale.
 * - Display TFT resistivo ILI9341 da 2,8 pollici, 320 x 240 pixel.
 * - Touchscreen, slot SD e audio disponibili sulla scheda di riferimento,
 *   ma non ancora inizializzati o usati dal firmware.
 * - L'eventuale gestione di una batteria dipende dalla revisione della scheda:
 *   non va presunta senza averne verificato schema e componenti.
 *
 * COLLEGAMENTI USATI OGGI
 * - TFT: MISO GPIO12, MOSI GPIO13, clock GPIO14, CS GPIO15, DC GPIO2,
 *   retroilluminazione GPIO21; i dettagli sono in platformio.ini.
 * - Sonde DS18B20: bus OneWire su GPIO27, da alimentare a 3,3 V con una
 *   resistenza di pull-up da 4,7 kOhm tra dati e 3,3 V.
 * - Pulsante BOOT: GPIO0 alterna oggi vista numerica e grafico; GPIO0
 *   partecipa anche alla selezione della modalita di avvio dell'ESP32.
 * - Le misure sono temporanee: fino a otto sonde, 120 campioni per sonda,
 *   circa due minuti di storico in RAM, senza salvataggio su SD.
 *
 * OBIETTIVO DEL PRODOTTO
 * L'operatore dovra trovare un'interfaccia semplice e usare il touch come
 * strumento operativo; il pulsante fisico restera utile secondo il flusso
 * che definiremo. Sono previsti piu tipi di sonde, da progettare in seguito.
 * All'avvio si vuole presentare una breve proposta grafica con testi di
 * dimensioni e colori diversi, seguita da un percorso guidato di tre tocchi
 * per familiarizzare l'operatore e verificare la calibrazione del touch.
 * Testi, grafica e modalita dei tre tocchi sono ancora da definire.
 *
 * STATO ATTUALE
 * Il firmware riconosce le DS18B20, mostra valori e grafico e usa BOOT per
 * cambiare vista. Touch, schermata introduttiva, guida iniziale e supporto
 * ad altri tipi di sonda non sono ancora implementati.
 * Ultima misura di compilazione PlatformIO (2026-10-31): 25.780 byte RAM su
 * 327.680 e 298.957 byte Flash su 1.310.720, secondo il riepilogo del build.
 *
 * Questo file mantiene visibile il flusso generale: prepara le periferiche,
 * poi controlla a ogni ciclo il pulsante, le sonde e lo schermo. I dettagli
 * sono raccolti nei moduli sotto include/ e vengono aggiunti esplicitamente
 * qui, secondo il metodo scelto per questo progetto.
 *
 * Ordine degli include del progetto:
 * 1. librerie necessarie ai moduli;
 * 2. costanti, stato condiviso e prototipi;
 * 3. frammenti di codice dei moduli effettivamente utilizzati.
 */
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <string.h>

#include "Constant.h"
#include "Var.h"
#include "Sub.h"

// Ogni frammento viene incluso una sola volta e non viene compilato separatamente da PlatformIO.
#include "devices/TemperatureSensors.cpp"
#include "devices/BootButton.cpp"
#include "devices/TemperatureDisplay.cpp"

/*
 * Avvia il firmware una sola volta dopo l'accensione o il reset della scheda.
 * La sequenza lascia per primi seriale e input, poi prepara il display e infine
 * cerca le sonde prima di mostrare la schermata iniziale.
 */
void setup() {
    Serial.begin(SERIAL_MONITOR_BAUD_RATE);
    initializeBootButton();
    initializeTemperatureDisplay();
    initializeTemperatureSensorBus();
    drawCurrentTemperatureScreen();
}

/*
 * Coordina un passaggio del funzionamento ripetuto del firmware.
 * Nessun passaggio attende a lungo: il pulsante resta reattivo anche durante
 * le conversioni delle sonde e lo schermo viene ridisegnato solo quando serve.
 */
void loop() {
    const uint32_t currentTimeMilliseconds = millis(); // Un unico riferimento temporale per questo passaggio del ciclo.

    updateBootButton(currentTimeMilliseconds);
    updateTemperatureMeasurements(currentTimeMilliseconds);

    if (temperatureDisplayNeedsRedraw) {
        drawCurrentTemperatureScreen();
    }
}
