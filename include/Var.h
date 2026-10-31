#ifndef ESP32_MULTI_PROBE_THERMOMETER_VAR_H
#define ESP32_MULTI_PROBE_THERMOMETER_VAR_H

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TFT_eSPI.h>

#include "Constant.h"

// Riunisce in un solo record i dati che appartengono alla stessa sonda.
struct TemperatureProbe {
    DeviceAddress romAddress; // Indirizzo ROM univoco a 64 bit che identifica la sonda sul bus OneWire.
    float latestTemperatureCelsius; // Ultima temperatura valida in gradi Celsius, oppure NAN se la lettura non e disponibile.
    float temperatureHistoryCelsius[TEMPERATURE_HISTORY_CAPACITY]; // Storico circolare delle letture in gradi Celsius.
};

OneWire ds18b20OneWireBus(DS18B20_DATA_PIN); // Gestisce la comunicazione elettrica OneWire sul GPIO delle sonde.
DallasTemperature ds18b20TemperatureSensors(&ds18b20OneWireBus); // Invia comandi alle sonde e legge le loro temperature.
TFT_eSPI temperatureDisplay; // Permette di disegnare testi, assi e curve sul display TFT.
TemperatureProbe temperatureProbes[MAXIMUM_PROBE_COUNT]; // Dati delle sonde riconosciute, dalla prima S1 all'ultima disponibile.

uint8_t activeTemperatureProbeCount = 0; // Numero di sonde utilizzabili, compreso tra zero e MAXIMUM_PROBE_COUNT.
uint16_t nextTemperatureHistoryWriteIndex = 0; // Posizione in cui verra scritto il prossimo campione dello storico circolare.
uint16_t storedTemperatureHistorySampleCount = 0; // Campioni gia presenti nello storico per ciascuna sonda.
uint32_t lastTemperatureConversionStartMilliseconds = 0; // Istante millis() in cui e stata avviata l'ultima conversione.
bool temperatureConversionIsPending = false; // Indica se una conversione e in corso e non e ancora pronta da leggere.
bool graphViewIsActive = false; // False mostra i valori numerici; true mostra il grafico temporale.
bool temperatureDisplayNeedsRedraw = true; // Richiede il ridisegno dello schermo dopo una nuova misura o un cambio vista.

#endif
