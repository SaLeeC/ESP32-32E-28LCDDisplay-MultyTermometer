/*
 * Progetto: SondAris (ESP32-32E-28LCDDisplay-MultyTermometer).
 * Scopo: misurare la temperatura di piu sonde e presentare i dati sul display.
 * Versione firmware: v1.034 01-11-2026.
 *
 * RISORSE DELLA SCHEDA DI RIFERIMENTO
 * - ESP32-WROOM-32E con Wi-Fi e Bluetooth disponibili; le funzioni wireless
 *   non sono utilizzate dalla versione attuale.
 * - Display TFT resistivo ILI9341 da 2,8 pollici, 320 x 240 pixel.
 * - Wiki della scheda di riferimento:
 *   https://www.lcdwiki.com/2.8inch_ESP32-32E_Display
 * - Touchscreen resistivo XPT2046: GPIO25 clock, GPIO32 MOSI, GPIO39 MISO,
 *   GPIO33 CS e GPIO36 interrupt, letto tramite linee software dedicate.
 * - Slot microSD: GPIO5 chip-select, GPIO18 clock, GPIO19 MISO e GPIO23 MOSI;
 *   dopo il logo la tabella mostra capacità e spazio residuo o errori FS.
 * - Audio disponibile sulla scheda di riferimento, ma non ancora usato.
 * - La tensione batteria viene letta su GPIO34 prima col carico minimo e poi
 *   con retroilluminazione, LED RGB e scansione WiFi alla massima potenza.
 *
 * COLLEGAMENTI USATI OGGI
 * - TFT: MISO GPIO12, MOSI GPIO13, clock GPIO14, CS GPIO15, DC GPIO2,
 *   retroilluminazione GPIO21; i dettagli sono in platformio.ini.
 * - Connettore sensori CN1: ricerca OneWire/DHT su GPIO27 e poi GPIO22;
 *   I2C prova SDA/SCL su GPIO27/22 e poi scambia i pin; una modalita per sessione.
 * - Sonde DS18B20: in modalita OneWire usare alimentazione a 3,3 V e una
 *   resistenza di pull-up da 4,7 kOhm tra il GPIO dati attivo e 3,3 V.
 * - Pulsante BOOT: GPIO0 alterna vista numerica e grafico ed e il recupero
 *   se il touch non e disponibile; partecipa anche alla modalita di avvio.
 * - Touch: tre bersagli all'avvio mostrano il rilevamento a colori e calcolano
 *   una calibrazione temporanea; BOOT puo escludere il touch in ogni momento.
 * - Il bus sensori CN1 viene verificato prima del logo e della microSD; il risultato
 *   OneWire/DHT/I2C e la mappatura GPIO appaiono nel riepilogo e nel menu tecnico.
 * - Se il filesystem microSD e utilizzabile,
 *   viene preparata /SondAris_Data. Non si creano file dati o si formatta.
 * - La barra inferiore consente di alternare Valore/Andamento e apre il menu
 *   tecnico tramite il pulsante ingranaggio.
 * - Le misure sono temporanee: fino a otto sonde, 120 campioni per sonda,
 *   circa due minuti OneWire o quattro minuti DHT in RAM, senza salvataggio.
 *
 * OBIETTIVO DEL PRODOTTO
 * L'operatore dovra trovare un'interfaccia semplice e usare il touch come
 * strumento operativo; il pulsante fisico restera utile secondo il flusso
 * che definiremo. Sono previsti piu tipi di sonde, da progettare in seguito.
 * All'avvio vengono provati OneWire, DHT e I2C, quindi l'intera ricerca viene
 * ripetuta scambiando GPIO27 e GPIO22 se non si rileva alcun dispositivo.
 * Il logo viene mostrato dopo la ricerca senza sospendere le misure gia attive.
 * Dopo la guida touch, l'operatore puo ripetere la ricerca con Start o BOOT.
 *
 * STATO ATTUALE
 * Il firmware controlla CN1 in sequenza OneWire, DHT e I2C, poi scambia i GPIO
 * se il primo ciclo non trova dispositivi; registra i ROM
 * OneWire, riconosce le DS18B20 e consente di scegliere DHT11 o DHT22 dopo il
 * rilevamento. Senza scelta DHT non misura. La schermata Start consente di
 * ripetere la scansione se entrambi i cicli sono vuoti. La calibrazione touch resta in RAM;
 * se il touch viene escluso con BOOT, la scelta DHT non e disponibile e le
 * misure restano disattivate. Lo switch del processo misure resta fermo con
 * zero sonde utilizzabili e parte appena almeno una sonda e configurata.
 * Ultima compilazione PlatformIO (2026-11-01): 50.828 byte RAM su 327.680 e
 * 914.253 byte Flash su 2.031.616, secondo il riepilogo del build.
 *
 * Questo file mantiene visibile il flusso generale: prepara le periferiche,
 * poi controlla a ogni ciclo touch, pulsante, sonde e schermo. I dettagli sono
 * raccolti nei moduli sotto include/ e vengono aggiunti esplicitamente qui,
 * secondo il metodo scelto per questo progetto.
 *
 * Ordine degli include del progetto:
 * 1. librerie necessarie ai moduli;
 * 2. costanti, stato condiviso e prototipi;
 * 3. frammenti di codice dei moduli effettivamente utilizzati.
 */
#include <Arduino.h>
#include <DallasTemperature.h>
#ifdef SENSOR_DRIVER_FOOTPRINT_PROBE
#define DHT12 ADAFRUIT_DHT12_SENSOR_TYPE
#endif
#include <DHT.h>
#ifdef SENSOR_DRIVER_FOOTPRINT_PROBE
#undef DHT12
#endif
#include <OneWire.h>
#include <SD.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <math.h>
#include <string.h>

#ifdef SENSOR_DRIVER_FOOTPRINT_PROBE
#include <Adafruit_AHTX0.h>
#include <Adafruit_AM2320.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_SHTC3.h>
#include <Adafruit_Si7021.h>
#include <ClosedCube_HDC1080.h>
#include <DHT12.h>
#include <SparkFunTMP102.h>
#include <SparkFun_TMP117.h>
#include <Temperature_LM75_Derived.h>
#include <bme68xLibrary.h>
#endif

#include "Constant.h"
#include "Var.h"
#include "Sub.h"

// Ogni frammento viene incluso una sola volta e non viene compilato separatamente da PlatformIO.
#include "devices/MicroSdCard.cpp"
#include "devices/ExternalI2cBus.cpp"
#include "devices/DhtSensors.cpp"
#include "devices/TemperatureSensors.cpp"
#include "devices/BootButton.cpp"
#include "devices/BatteryMonitor.cpp"
#include "devices/TemperatureDisplay.cpp"
#include "devices/Touchscreen.cpp"
#ifdef SENSOR_DRIVER_FOOTPRINT_PROBE
#include "devices/SensorDriverFootprint.cpp"
#endif

/*
 * Avvia il firmware una sola volta dopo l'accensione o il reset della scheda.
 * La sequenza avvia seriale, pulsante e display, controlla CN1, mostra il logo,
 * controlla la microSD e guida touch, scelta DHT ed eventuale retry sensori.
 * Le attese successive continuano a servire ogni processo di misura gia attivo.
 */
void setup() {
    Serial.begin(SERIAL_MONITOR_BAUD_RATE);
#ifdef SENSOR_DRIVER_FOOTPRINT_PROBE
    retainAllSensorDriverCodeForFootprintMeasurement();
#endif
    initializeBootButton();
    initializeTemperatureDisplay();
    initializeTouchscreen();
    startupBatteryCheckResult = measureStartupBatteryVoltage();
    initializeTemperatureSensorBus();
    drawStartupPresentation();
    waitWhileServicingMeasurements(
        static_cast<uint32_t>(SONDARIS_STARTUP_LOGO_DURATION_SECONDS) *
        1000UL);

    startupMicroSdCardCheckResult = verifyMicroSdCardAndPrepareDataDirectory();
    drawStartupSummaryScreen(
        startupMicroSdCardCheckResult,
        startupExternalSensorBusCheckResult);
    waitWhileServicingMeasurements(
        static_cast<uint32_t>(STARTUP_SUMMARY_SCREEN_DURATION_SECONDS) *
        1000UL);

    runStartupTouchExperience(); // Dopo il riepilogo guida i tre tocchi e consente il recupero con BOOT.
    runStartupDhtSensorModelSelection(); // Il DHT viene configurato solo dopo la scelta esplicita dell'operatore.
    runStartupNoSensorRecovery(); // Se entrambi i cicli sono vuoti attende Start o BOOT e ripete la ricerca.
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
    updateTouchscreen();
    updateTemperatureMeasurements(currentTimeMilliseconds);

    if (temperatureDisplayNeedsRedraw) {
        drawCurrentTemperatureScreen();
    }
}
