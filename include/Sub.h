#ifndef ESP32_MULTI_PROBE_THERMOMETER_SUB_H
#define ESP32_MULTI_PROBE_THERMOMETER_SUB_H

#include <Arduino.h>

#include "Constant.h"

/*
 * Riassume cio che si puo dedurre dalla sola tensione BAT_ADC presente su GPIO34.
 */
enum class StartupPowerAssessment : uint8_t {
    UsbWithoutDetectedBattery, // La scheda e accesa ma sul ramo batteria non compare una tensione da LiPo.
    BatteryNodeVoltageDetected, // BAT+ e plausibile, ma non dimostra la presenza della batteria ne distingue la fonte.
    BatteryVoltageOutOfRange // La tensione calcolata non e compatibile con il circuito documentato.
};

/*
 * Conserva la media ADC e la tensione batteria ricostruita dal partitore 100 kOhm/100 kOhm.
 */
struct StartupBatteryCheckResult {
    uint16_t adcVoltageMillivolts; // Tensione media letta direttamente su GPIO34, in millivolt.
    uint16_t batteryVoltageMillivolts; // Tensione stimata prima del partitore, in millivolt.
    uint16_t maximumLoadBatteryVoltageMillivolts; // Tensione BAT+ con retroilluminazione massima e scansione WiFi attiva, in millivolt.
    int16_t loadVoltageChangeMillivolts; // Differenza carico massimo meno carico minimo; un valore negativo indica una caduta.
    bool loadTestWasCompleted; // True se retroilluminazione e radio hanno applicato tutto il carico previsto.
    StartupPowerAssessment powerAssessment; // Deduzione prudente ottenuta dalla tensione misurata.
};

/*
 * Distingue la scheda non raggiungibile, l'errore del filesystem e il montaggio riuscito.
 */
enum class MicroSdCardCheckStatus : uint8_t {
    NotDetected, // Nessuna risposta valida della scheda durante l'inizializzazione SPI.
    FileSystemError, // La scheda risponde, ma il filesystem non puo essere montato o letto.
    Ready // La scheda e montata e capacita e spazio residuo sono disponibili.
};

/*
 * Riporta se la cartella dati era gia presente, e stata creata o non e utilizzabile.
 */
enum class MicroSdDataDirectoryStatus : uint8_t {
    NotChecked, // Il filesystem non era disponibile per verificare la cartella.
    AlreadyPresent, // La cartella dati esisteva gia nella radice della scheda.
    Created, // La cartella dati e stata creata e verificata durante l'avvio.
    CreationError // La cartella non puo essere usata o creata.
};

/*
 * Riporta lo stato della microSD, lo spazio utilizzabile e l'esito della cartella dati.
 */
struct MicroSdCardCheckResult {
    MicroSdCardCheckStatus status; // Stato ottenuto durante il controllo di avvio.
    uint64_t totalCapacityBytes; // Capacita totale del filesystem montato, in byte.
    uint64_t freeSpaceBytes; // Spazio residuo del filesystem montato, in byte.
    MicroSdDataDirectoryStatus dataDirectoryStatus; // Stato della cartella /SondAris_Data.
};

/*
 * Indica quale modalita alternativa e stata selezionata sul connettore CN1.
 */
enum class ExternalSensorBusMode : uint8_t {
    NotChecked, // Il controllo non e ancora stato eseguito.
    OneWire, // Sono stati rilevati uno o piu dispositivi OneWire sul GPIO selezionato.
    Dht, // E stato verificato un sensore DHT single-wire sul GPIO selezionato.
    I2c, // Il bus e stato riconfigurato e scansionato sui GPIO SDA/SCL selezionati.
    I2cInitializationError, // La riconfigurazione o l'avvio del bus I2C non e riuscito.
    NoSensorDetected // Nessun protocollo ha rilevato dispositivi in entrambe le configurazioni GPIO.
};

/*
 * Indica se il processo periodico di acquisizione deve restare fermo o lavorare.
 */
enum class MeasurementProcessState : uint8_t {
    StoppedNoUsableSensors, // Nessuna sonda leggibile e configurata: non vengono richieste misure.
    Running // Almeno una sonda e utilizzabile: le misure periodiche sono abilitate.
};

/*
 * Rappresenta l'assegnazione standard o scambiata dei due GPIO del connettore CN1.
 */
enum class ExternalSensorPinMapping : uint8_t {
    Standard, // OneWire/DHT su GPIO27; I2C SDA su GPIO27 e SCL su GPIO22.
    Swapped // OneWire/DHT su GPIO22; I2C SDA su GPIO22 e SCL su GPIO27.
};

/*
 * Conserva protocollo, conteggi, indirizzi e mappatura GPIO per riepilogo e menu.
 */
struct ExternalSensorBusCheckResult {
    ExternalSensorBusMode mode; // Modalita attiva dopo il controllo del connettore CN1.
    uint8_t oneWireDeviceCount; // Dispositivi trovati sul bus OneWire prima dell'eventuale commutazione.
    uint8_t i2cDeviceCount; // Dispositivi che hanno risposto alla scansione I2C.
    uint8_t i2cDeviceAddresses[MAXIMUM_EXTERNAL_I2C_DEVICE_COUNT]; // Indirizzi I2C trovati, nell'intervallo standard valido.
    DhtSensorModel dhtModel; // Modello scelto dall'operatore, oppure NotSelected finche le misure restano disattivate.
    ExternalSensorPinMapping pinMapping; // Configurazione GPIO che ha permesso il rilevamento o l'ultimo tentativo.
};

/*
 * Prepara GPIO0 come ingresso con resistenza interna di richiamo.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: configura il pin del pulsante BOOT, che vale LOW quando viene premuto.
 */
void initializeBootButton();

/*
 * Acquisisce e media la tensione BAT_ADC disponibile sulla scheda.
 * Parametri: nessuno.
 * Restituisce: misure a WiFi spento e acceso, loro differenza e valutazione preliminare.
 * Effetti: configura GPIO34; confronta retroilluminazione, LED RGB e WiFi
 * spenti con retroilluminazione massima, tutti i canali RGB accesi e scansione
 * radio alla massima potenza; infine rilascia i GPIO del LED, spegne la radio,
 * lascia la retroilluminazione accesa e scrive la diagnosi.
 */
StartupBatteryCheckResult measureStartupBatteryVoltage();

/*
 * Controlla se BOOT e stato premuto in modo stabile e alterna la vista.
 * Parametri: currentTimeMilliseconds e il valore attuale di millis(), in millisecondi.
 * Restituisce: nulla.
 * Effetti: puo cambiare la vista attiva e richiedere il ridisegno del display.
 */
void updateBootButton(uint32_t currentTimeMilliseconds);

/*
 * Configura il controller touch sui pin dedicati della scheda.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: imposta risoluzione e orientamento iniziali; la calibrazione si svolge separatamente.
 */
void initializeTouchscreen();

/*
 * Guida tre tocchi che verificano e calibrano il touchscreen all'avvio.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: salva in RAM la trasformazione touch-display; BOOT permette il recupero senza touch.
 */
void runStartupTouchExperience();

/*
 * Chiede all'operatore il modello del DHT rilevato, se il touch e calibrato.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: configura DHT11 o DHT22 solo dopo la scelta; timeout, BOOT o touch
 * non calibrato lasciano il modello non selezionato e le misure disattivate.
 */
void runStartupDhtSensorModelSelection();

/*
 * Attende Start o BOOT quando entrambe le configurazioni non trovano sensori.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: ripete la scansione; se rileva un DHT, propone la scelta del modello.
 */
void runStartupNoSensorRecovery();

/*
 * Legge i tocchi durante il normale funzionamento e attiva il comando sul display.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: puo alternare la vista quando viene premuto il pulsante touch disegnato a schermo.
 */
void updateTouchscreen();

/*
 * Inizializza il display e accende la retroilluminazione.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: configura la periferica TFT e la sua orientazione orizzontale.
 */
void initializeTemperatureDisplay();

/*
 * Ridisegna il display mostrando la vista selezionata o il menu tecnico.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: cancella e aggiorna lo schermo; azzera la richiesta di ridisegno.
 */
void drawCurrentTemperatureScreen();

/*
 * Disegna il logo SondAris, una breve descrizione e la versione firmware.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: occupa il display durante la presentazione iniziale; non modifica i dati delle sonde.
 */
void drawStartupPresentation();

/*
 * Mostra nome, versione, esito del bus CN1 e capacità/spazio residuo microSD.
 * Parametri: microSdCardCheckResult contiene i dati SD; externalSensorBusCheckResult
 * contiene la modalita OneWire/I2C e gli indirizzi eventualmente rilevati.
 * Restituisce: nulla.
 * Effetti: sostituisce la schermata corrente; non modifica file o dati delle sonde.
 */
void drawStartupSummaryScreen(
    const MicroSdCardCheckResult& microSdCardCheckResult,
    const ExternalSensorBusCheckResult& externalSensorBusCheckResult);

/*
 * Mostra l'assenza di sensori con un pulsante Start e il recupero BOOT.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna il display senza modificare lo stato del bus.
 */
void drawStartupNoSensorScreen();

/*
 * Mostra che la nuova scansione dei due GPIO e in corso.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna il display senza modificare lo stato del bus.
 */
void drawStartupSensorSearchProgressScreen();

/*
 * Disegna la schermata che propone DHT11 e DHT22 all'operatore.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna il display senza avviare misure.
 */
void drawDhtSensorModelSelectionScreen();

/*
 * Mostra l'esito della scelta del modello DHT.
 * Parametri: selectionResultMessage e il messaggio da mostrare.
 * Restituisce: nulla.
 * Effetti: aggiorna temporaneamente il display senza modificare il sensore.
 */
void drawDhtSensorModelSelectionResult(const char* selectionResultMessage);

/*
 * Mostra uno dei tre bersagli in attesa del tocco.
 * Parametri: calibrationTargetIndex e l'indice del bersaglio, da zero a due.
 * Restituisce: nulla.
 * Effetti: disegna istruzioni, avanzamento, bersaglio e promemoria per BOOT.
 */
void drawTouchCalibrationTarget(uint8_t calibrationTargetIndex);

/*
 * Mostra l'esito visivo del contatto con il bersaglio corrente.
 * Parametri: calibrationTargetIndex identifica il punto; feedbackMessage e
 * feedbackColor descrivono l'esito; calibrationPointWasCaptured indica se il
 * punto va segnato come acquisito.
 * Restituisce: nulla.
 * Effetti: aggiorna colori e messaggio senza modificare la calibrazione.
 */
void drawTouchCalibrationTargetFeedback(
    uint8_t calibrationTargetIndex,
    const char* feedbackMessage,
    uint16_t feedbackColor,
    bool calibrationPointWasCaptured);

/*
 * Spiega il motivo per cui un singolo bersaglio non e stato acquisito.
 * Parametri: calibrationTargetIndex identifica il bersaglio; touchWasDetected
 * distingue una lettura non valida da un tocco non rilevato entro il timeout.
 * Restituisce: nulla.
 * Effetti: invita a ripresentare il solo punto fallito oppure a usare BOOT.
 */
void drawTouchCalibrationTargetRetryPrompt(
    uint8_t calibrationTargetIndex,
    bool touchWasDetected);

/*
 * Conferma che il touch e stato calibrato per l'accensione corrente.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: mostra un messaggio di conferma sul display.
 */
void drawTouchCalibrationSuccess();

/*
 * Chiede di ripetere tutti i punti quando la geometria della calibrazione non e valida.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: mostra le opzioni per ripetere l'intero test o proseguire con BOOT.
 */
void drawTouchCalibrationRecoveryPrompt();

/*
 * Avvisa che si e proseguito senza calibrare il touch.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: mostra la modalita di recupero basata sul pulsante BOOT.
 */
void drawTouchCalibrationBypassed();

/*
 * Disegna la barra inferiore con cambio vista e pulsante ingranaggio.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: mostra l'azione Valore/Andamento successiva e lo stato del menu tecnico.
 */
void drawControlBar();

/*
 * Mostra le informazioni tecniche disponibili durante la sessione corrente.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: disegna versione, sonde, touch, bus CN1, stato microSD e cartella dati.
 */
void drawTechnicalMenuScreen();

/*
 * Verifica la microSD e prepara la cartella dati se il filesystem e utilizzabile.
 * Parametri: nessuno.
 * Restituisce: stato del controllo, capacità e spazio libero in byte, oltre allo stato cartella.
 * Effetti: inizializza SPI e puo creare solo /SondAris_Data; non crea file dati o formatta.
 */
MicroSdCardCheckResult verifyMicroSdCardAndPrepareDataDirectory();

/*
 * Disegna una riga numerica per ogni sonda riconosciuta.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: scrive sul display e mostra "--" per una temperatura non disponibile.
 */
void drawNumericTemperatureValues();

/*
 * Trova la temperatura minima e massima valide conservate nello storico.
 * Parametri: i due riferimenti ricevono gli estremi in gradi Celsius.
 * Restituisce: true se esiste almeno una misura valida, altrimenti false.
 * Effetti: legge gli storici senza modificarli.
 */
bool findValidTemperatureHistoryRange(float& lowestValidTemperatureCelsius,
                                      float& highestValidTemperatureCelsius);

/*
 * Disegna gli assi del grafico e le etichette del tempo.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: scrive linee ed etichette temporali sul display.
 */
void drawTemperatureGraphAxes();

/*
 * Disegna i limiti superiore e inferiore della scala verticale.
 * Parametri: temperature minima e massima gia arrotondate, in gradi Celsius.
 * Restituisce: nulla.
 * Effetti: scrive sul display i due estremi della scala.
 */
void drawTemperatureGraphTemperatureAxisLabels(float lowestAxisTemperatureCelsius,
                                               float highestAxisTemperatureCelsius);

/*
 * Converte la posizione cronologica di un campione nell'indice del buffer circolare.
 * Parametri: chronologicalSampleOffset e l'indice del campione dal piu vecchio al piu recente.
 * Restituisce: indice fisico compreso tra zero e TEMPERATURE_HISTORY_CAPACITY - 1.
 * Effetti: nessuno; presuppone che l'indice cronologico sia presente nello storico.
 */
uint16_t calculateChronologicalHistoryBufferIndex(uint16_t chronologicalSampleOffset);

/*
 * Disegna la curva dello storico per una singola sonda.
 * Parametri: probeIndex e l'indice della sonda da zero; gli altri due valori sono i limiti della scala in gradi Celsius.
 * Restituisce: nulla.
 * Effetti: disegna punti e segmenti; una misura non valida interrompe la curva.
 */
void drawTemperatureHistoryForProbe(uint8_t probeIndex,
                                    float lowestAxisTemperatureCelsius,
                                    float highestAxisTemperatureCelsius);

/*
 * Disegna il grafico completo, la scala verticale e la legenda delle sonde.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: legge lo storico e disegna sul display; non modifica le temperature.
 */
void drawTemperatureHistoryGraph();

/*
 * Disegna il nome e l'ultima lettura disponibile per ogni sonda sotto il grafico.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: scrive la legenda sul display.
 */
void drawTemperatureGraphLegend();

/*
 * Controlla OneWire, poi DHT e infine I2C solo se i primi due protocolli sono assenti.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: prova OneWire/DHT/I2C in esclusiva e scambia i GPIO se non trova dispositivi.
 */
void initializeTemperatureSensorBus();

/*
 * Allinea il processo di misura al numero di sonde effettivamente utilizzabili.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: con zero sonde arresta le acquisizioni; con almeno una sonda avvia
 * subito la prima acquisizione prevista dal protocollo attivo.
 */
void updateMeasurementProcessState();

/*
 * Attende senza sospendere il processo di acquisizione gia attivo.
 * Parametri: durationMilliseconds e la durata complessiva dell'attesa.
 * Restituisce: nulla.
 * Effetti: richiama periodicamente il processo misure e cede tempo al sistema.
 */
void waitWhileServicingMeasurements(uint32_t durationMilliseconds);

/*
 * Verifica sul filo dati CN1 una risposta DHT completa, compreso il checksum.
 * Parametri: dataPin e il GPIO sul quale provare il protocollo DHT.
 * Restituisce: true se il GPIO ha restituito un frame DHT valido.
 * Effetti: genera il segnale di avvio DHT e poi rilascia il pin.
 */
bool detectDhtSensorOnExternalBus(uint8_t dataPin);

/*
 * Configura il driver del modello DHT scelto dall'operatore.
 * Parametri: selectedModel deve essere DHT11 o DHT22; dataPin e il GPIO rilevato.
 * Restituisce: true se il modello e supportato.
 * Effetti: inizializza il driver e rende disponibile una sola sonda termica.
 */
bool initializeSelectedDhtSensor(DhtSensorModel selectedModel, uint8_t dataPin);

/*
 * Restituisce il nome breve del modello per le schermate.
 * Parametri: dhtSensorModel e il modello da descrivere.
 * Restituisce: nome statico del modello o testo di stato non selezionato.
 * Effetti: nessuno.
 */
const char* getDhtSensorShortModelName(DhtSensorModel dhtSensorModel);

/*
 * Aggiorna temperatura e umidita DHT senza superare la frequenza supportata.
 * Parametri: currentTimeMilliseconds e il valore attuale di millis(), in millisecondi.
 * Restituisce: nulla.
 * Effetti: salva l'ultima lettura e lo storico; senza modello selezionato non misura.
 */
void updateDhtSensorMeasurements(uint32_t currentTimeMilliseconds);

/*
 * Cerca le sonde presenti e inizializza i dati di quelle che il firmware puo gestire.
 * Parametri: nessuno.
 * Restituisce: numero complessivo di dispositivi trovati sul bus OneWire, anche se non sono DS18B20.
 * Effetti: aggiorna l'elenco globale delle sonde e segnala eventuali dispositivi oltre il limite.
 */
uint8_t discoverTemperatureProbes();

/*
 * Riconfigura CN1 come I2C e cerca i dispositivi agli indirizzi standard.
 * Parametri: externalSensorBusCheckResult riceve stato e indirizzi trovati.
 * Restituisce: nulla.
 * Effetti: rilascia i GPIO configurati, inizializza Wire a 100 kHz e lascia il bus in modalita I2C.
 */
void initializeExternalI2cBusAndScan(
    ExternalSensorBusCheckResult& externalSensorBusCheckResult);

/*
 * Restituisce il GPIO dati OneWire/DHT per la mappatura selezionata.
 * Parametri: pinMapping e la configurazione standard o scambiata.
 * Restituisce: GPIO27 o GPIO22.
 * Effetti: nessuno.
 */
uint8_t getExternalSensorSingleWireDataPin(ExternalSensorPinMapping pinMapping);

/*
 * Restituisce il GPIO SDA per la mappatura I2C selezionata.
 * Parametri: pinMapping e la configurazione standard o scambiata.
 * Restituisce: GPIO27 o GPIO22.
 * Effetti: nessuno.
 */
uint8_t getExternalSensorI2cSdaPin(ExternalSensorPinMapping pinMapping);

/*
 * Restituisce il GPIO SCL per la mappatura I2C selezionata.
 * Parametri: pinMapping e la configurazione standard o scambiata.
 * Restituisce: GPIO27 o GPIO22.
 * Effetti: nessuno.
 */
uint8_t getExternalSensorI2cSclPin(ExternalSensorPinMapping pinMapping);
/*
 * Legge le conversioni terminate e avvia la misura successiva quando e il momento.
 * Parametri: currentTimeMilliseconds e il valore attuale di millis(), in millisecondi.
 * Restituisce: nulla.
 * Effetti: aggiorna temperature, storico circolare e richiesta di ridisegno senza bloccare il programma.
 */
void updateTemperatureMeasurements(uint32_t currentTimeMilliseconds);

/*
 * Aggiunge allo storico circolare l'ultima lettura di ogni sonda attiva.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: avanza l'indice comune e richiede il ridisegno della vista se necessario.
 */
void storeTemperatureHistorySample();

#ifdef SENSOR_DRIVER_FOOTPRINT_PROBE
/*
 * Collega nel firmware di prova i percorsi principali di tutti i driver sensore.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: il ramo resta disattivato sulla scheda, ma impedisce al linker di
 * eliminare inizializzazione e lettura durante la misura dell'occupazione.
 */
void retainAllSensorDriverCodeForFootprintMeasurement();
#endif

/*
 * Punto di avvio Arduino: prepara le periferiche, guida l'operatore sul touch e avvia le misure.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: avvia seriale, pulsante, display, touch e bus delle sonde; mostra la schermata operativa.
 */
void setup();

/*
 * Ciclo principale Arduino: controlla touch, pulsante, misure e aggiornamento dello schermo.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: coordina i moduli e puo cambiare la vista una volta per ogni ciclo del microcontrollore.
 */
void loop();

#endif
