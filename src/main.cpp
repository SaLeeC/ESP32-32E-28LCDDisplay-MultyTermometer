/*
 * Punto d'ingresso e coordinamento del termometro multiplo.
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
