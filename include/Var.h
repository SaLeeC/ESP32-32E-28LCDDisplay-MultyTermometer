#ifndef ESP32_MULTI_PROBE_THERMOMETER_VAR_H
#define ESP32_MULTI_PROBE_THERMOMETER_VAR_H

#include <Arduino.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <OneWire.h>
#include <TFT_eSPI.h>
#include <TFT_Touch.h>

#include "Constant.h"
#include "Sub.h"

// Riunisce in un solo record i dati che appartengono alla stessa sonda.
struct TemperatureProbe {
    DeviceAddress romAddress; // Indirizzo ROM univoco a 64 bit che identifica la sonda sul bus OneWire.
    float latestTemperatureCelsius; // Ultima temperatura valida in gradi Celsius, oppure NAN se la lettura non e disponibile.
    float temperatureHistoryCelsius[TEMPERATURE_HISTORY_CAPACITY]; // Storico circolare delle letture in gradi Celsius.
};

// Collega un indirizzo I2C rilevato alle voci candidate del catalogo sensori.
struct ExternalI2cSensorRecord {
    uint8_t i2cAddress; // Indirizzo del dispositivo sul bus I2C, su 7 bit.
    uint8_t catalogCandidateCount; // Numero di voci del catalogo compatibili con l'indirizzo, da 0 al numero di voci.
    const ExternalI2cSensorCatalogEntry*
        catalogCandidateEntries[EXTERNAL_I2C_SENSOR_CATALOG_ENTRY_COUNT]; // Riferimenti alle famiglie candidate, non ancora identificate.
    const ExternalI2cSensorCatalogEntry*
        identifiedCharacteristics; // Voce selezionata solo dopo aver verificato il protocollo di identificazione.
};

// Collega il ROM completo di un dispositivo OneWire alle famiglie compatibili.
struct OneWireSensorRecord {
    DeviceAddress romAddress; // Identificativo ROM del dispositivo, composto da 8 byte.
    uint8_t catalogCandidateCount; // Numero di voci compatibili con il codice famiglia ROM, da 0 al numero di voci.
    const OneWireSensorCatalogEntry*
        catalogCandidateEntries[ONE_WIRE_SENSOR_CATALOG_ENTRY_COUNT]; // Riferimenti alle famiglie candidate del catalogo OneWire.
    const OneWireSensorCatalogEntry*
        identifiedCharacteristics; // Voce specifica selezionata solo dopo una verifica ulteriore del dispositivo.
};

// Trasforma le coordinate grezze XPT2046 nelle coordinate in pixel del display.
struct TouchScreenCalibrationTransform {
    float screenXFromRawX; // Quanto cambia X schermo quando cambia X grezzo del touch.
    float screenXFromRawY; // Quanto cambia X schermo quando cambia Y grezzo del touch.
    float screenXOffset; // Correzione iniziale della coordinata X sul display.
    float screenYFromRawX; // Quanto cambia Y schermo quando cambia X grezzo del touch.
    float screenYFromRawY; // Quanto cambia Y schermo quando cambia Y grezzo del touch.
    float screenYOffset; // Correzione iniziale della coordinata Y sul display.
};

OneWire ds18b20OneWireBus(DS18B20_DATA_PIN); // Gestisce la comunicazione elettrica OneWire sul GPIO delle sonde.
DallasTemperature ds18b20TemperatureSensors(&ds18b20OneWireBus); // Invia comandi alle sonde e legge le loro temperature.
TFT_eSPI temperatureDisplay; // Permette di disegnare testi, assi e curve sul display TFT.
DHT dht11ExternalSensorOnPrimaryPin(EXTERNAL_SENSOR_PRIMARY_GPIO_PIN, DHT11); // Driver DHT11 per il primo tentativo su GPIO27.
DHT dht22ExternalSensorOnPrimaryPin(EXTERNAL_SENSOR_PRIMARY_GPIO_PIN, DHT22); // Driver DHT22 per il primo tentativo su GPIO27.
DHT dht11ExternalSensorOnSecondaryPin(EXTERNAL_SENSOR_SECONDARY_GPIO_PIN, DHT11); // Driver DHT11 per il tentativo alternativo su GPIO22.
DHT dht22ExternalSensorOnSecondaryPin(EXTERNAL_SENSOR_SECONDARY_GPIO_PIN, DHT22); // Driver DHT22 per il tentativo alternativo su GPIO22.
DHT* activeDhtExternalSensor = nullptr; // Driver DHT sul GPIO effettivamente rilevato, nullo finche il modello non e scelto.
TFT_Touch resistiveTouchController(
    XPT2046_TOUCH_CHIP_SELECT_PIN,
    XPT2046_TOUCH_CLOCK_PIN,
    XPT2046_TOUCH_MOSI_PIN,
    XPT2046_TOUCH_MISO_PIN); // Legge il controller resistivo tramite le sue linee dedicate.
TemperatureProbe temperatureProbes[MAXIMUM_PROBE_COUNT]; // Dati delle sonde riconosciute, dalla prima S1 all'ultima disponibile.
ExternalI2cSensorRecord externalI2cSensorRecords[MAXIMUM_EXTERNAL_I2C_SENSOR_COUNT] = {}; // Registro statico di al massimo otto dispositivi I2C e dei loro candidati catalogo.
OneWireSensorRecord oneWireSensorRecords[MAXIMUM_ONEWIRE_SENSOR_COUNT] = {}; // Registro statico di al massimo otto ROM OneWire e dei loro candidati catalogo.

uint8_t activeTemperatureProbeCount = 0; // Numero di sonde utilizzabili, compreso tra zero e MAXIMUM_PROBE_COUNT.
uint8_t externalI2cSensorRecordCount = 0; // Numero di dispositivi I2C memorizzati nel registro, compreso tra zero e MAXIMUM_EXTERNAL_I2C_SENSOR_COUNT.
uint8_t oneWireSensorRecordCount = 0; // Numero di dispositivi OneWire memorizzati, compreso tra zero e MAXIMUM_ONEWIRE_SENSOR_COUNT.
uint16_t nextTemperatureHistoryWriteIndex = 0; // Posizione in cui verra scritto il prossimo campione dello storico circolare.
uint16_t storedTemperatureHistorySampleCount = 0; // Campioni gia presenti nello storico per ciascuna sonda.
uint32_t lastTemperatureConversionStartMilliseconds = 0; // Istante millis() in cui e stata avviata l'ultima conversione.
bool temperatureConversionIsPending = false; // Indica se una conversione e in corso e non e ancora pronta da leggere.
MeasurementProcessState measurementProcessState =
    MeasurementProcessState::StoppedNoUsableSensors; // Mantiene fermo il processo finche non esiste almeno una sonda configurata.
bool graphViewIsActive = false; // False mostra i valori numerici; true mostra il grafico temporale.
bool technicalMenuIsActive = false; // True mostra il menu tecnico aperto dal pulsante ingranaggio.
bool temperatureDisplayNeedsRedraw = true; // Richiede il ridisegno dello schermo dopo una nuova misura o un cambio vista.
StartupBatteryCheckResult startupBatteryCheckResult = { // Misura del ramo batteria eseguita una sola volta durante l'accensione.
    0,
    0,
    0,
    0,
    false,
    StartupPowerAssessment::BatteryVoltageOutOfRange
};
MicroSdCardCheckResult startupMicroSdCardCheckResult = { // Stato microSD conservato per riepilogo di avvio e menu tecnico.
    MicroSdCardCheckStatus::NotDetected,
    0,
    0,
    MicroSdDataDirectoryStatus::NotChecked
};
ExternalSensorBusCheckResult startupExternalSensorBusCheckResult = { // Risultato del controllo OneWire/DHT/I2C conservato per tabella e menu tecnico.
    ExternalSensorBusMode::NotChecked,
    0,
    0,
    {0},
    DhtSensorModel::NotSelected,
    ExternalSensorPinMapping::Standard
};
bool externalI2cBusIsInitialized = false; // Indica se Wire deve essere terminato prima di riconfigurare i GPIO CN1.
float latestDhtRelativeHumidityPercent = NAN; // Ultima umidita relativa DHT valida, in percentuale, oppure NAN se assente.
uint32_t lastDhtMeasurementMilliseconds = 0; // Istante dell'ultima acquisizione DHT, in millisecondi.
bool dhtMeasurementHasBeenAttempted = false; // Evita di attendere l'intervallo prima della prima lettura del modello selezionato.
TouchScreenCalibrationTransform resistiveTouchCalibration = {}; // Parametri calcolati dai tre bersagli toccati all'avvio.
bool resistiveTouchCalibrationIsValid = false; // False impedisce di usare coordinate touch non calibrate.
bool resistiveTouchPressHasBeenHandled = false; // Evita piu attivazioni mentre l'operatore tiene il dito sullo schermo.

#endif
