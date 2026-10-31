#ifndef ESP32_MULTI_PROBE_THERMOMETER_SUB_H
#define ESP32_MULTI_PROBE_THERMOMETER_SUB_H

#include <Arduino.h>

/*
 * Prepara GPIO0 come ingresso con resistenza interna di richiamo.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: configura il pin del pulsante BOOT, che vale LOW quando viene premuto.
 */
void initializeBootButton();

/*
 * Controlla se BOOT e stato premuto in modo stabile e alterna la vista.
 * Parametri: currentTimeMilliseconds e il valore attuale di millis(), in millisecondi.
 * Restituisce: nulla.
 * Effetti: puo cambiare la vista attiva e richiedere il ridisegno del display.
 */
void updateBootButton(uint32_t currentTimeMilliseconds);

/*
 * Inizializza il display e accende la retroilluminazione.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: configura la periferica TFT e la sua orientazione orizzontale.
 */
void initializeTemperatureDisplay();

/*
 * Ridisegna il display scegliendo messaggio, valori o grafico in base allo stato.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: cancella e aggiorna lo schermo; azzera la richiesta di ridisegno.
 */
void drawCurrentTemperatureScreen();

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
 * Avvia il bus OneWire, trova le DS18B20 e prepara la prima conversione.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna i dati globali delle sonde e puo stampare gli indirizzi seriali.
 */
void initializeTemperatureSensorBus();

/*
 * Cerca le sonde presenti e inizializza i dati di quelle che il firmware puo gestire.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna l'elenco globale delle sonde e segnala eventuali dispositivi oltre il limite.
 */
void discoverTemperatureProbes();

/*
 * Legge le conversioni terminate e avvia la misura successiva quando e il momento.
 * Parametri: currentTimeMilliseconds e il valore attuale di millis(), in millisecondi.
 * Restituisce: nulla.
 * Effetti: aggiorna temperature, storico circolare e richiesta di ridisegno senza bloccare il programma.
 */
void updateTemperatureMeasurements(uint32_t currentTimeMilliseconds);

/*
 * Punto di avvio Arduino: prepara le periferiche e mostra la schermata iniziale.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: avvia seriale, pulsante, display e bus delle sonde.
 */
void setup();

/*
 * Ciclo principale Arduino: controlla pulsante, misure e aggiornamento dello schermo.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: coordina i moduli una volta per ogni ciclo del microcontrollore.
 */
void loop();

#endif
