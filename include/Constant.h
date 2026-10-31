#ifndef ESP32_MULTI_PROBE_THERMOMETER_CONSTANT_H
#define ESP32_MULTI_PROBE_THERMOMETER_CONSTANT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// Configurazione del bus e delle sonde DS18B20
constexpr uint32_t SERIAL_MONITOR_BAUD_RATE = 115200; // Velocita del monitor seriale, espressa in bit al secondo.
constexpr uint8_t DS18B20_DATA_PIN = 27; // GPIO collegato al filo dati comune delle sonde DS18B20.
constexpr uint8_t MAXIMUM_PROBE_COUNT = 8; // Numero massimo di sonde che il firmware puo gestire.
constexpr uint8_t DS18B20_FAMILY_CODE = 0x28; // Codice nel ROM che identifica una sonda della famiglia DS18B20.
constexpr uint8_t DS18B20_RESOLUTION_BITS = 12; // Risoluzione usata per la misura; a 12 bit servono fino a 750 ms.
constexpr float DS18B20_MINIMUM_TEMPERATURE_CELSIUS = -55.0f; // Limite inferiore valido dichiarato per una DS18B20, in gradi Celsius.
constexpr float DS18B20_MAXIMUM_TEMPERATURE_CELSIUS = 125.0f; // Limite superiore valido dichiarato per una DS18B20, in gradi Celsius.
constexpr uint32_t DS18B20_CONVERSION_DURATION_MILLISECONDS = 750; // Attesa necessaria prima di leggere una conversione a 12 bit.
constexpr uint32_t TEMPERATURE_SAMPLE_INTERVAL_MILLISECONDS = 1000; // Intervallo tra l'avvio di due misure, in millisecondi.
constexpr uint16_t TEMPERATURE_HISTORY_CAPACITY = 120; // Campioni conservati per sonda; uno al secondo copre circa due minuti.

// Configurazione del pulsante BOOT
constexpr uint8_t BOOT_BUTTON_PIN = 0; // GPIO0 del pulsante BOOT, che vale LOW quando viene premuto.
constexpr uint32_t BOOT_BUTTON_DEBOUNCE_DURATION_MILLISECONDS = 40; // Stabilita richiesta per ignorare il rimbalzo elettrico del pulsante.

// Configurazione generale del display
constexpr uint8_t TEMPERATURE_DISPLAY_LANDSCAPE_ROTATION = 1; // Rotazione orizzontale usata dal display 320 x 240.
constexpr uint8_t TEMPERATURE_DISPLAY_TITLE_FONT = 2; // Font TFT_eSPI per titolo e valori numerici.
constexpr uint8_t TEMPERATURE_DISPLAY_GRAPH_FONT = 1; // Font TFT_eSPI piu piccolo, usato nel grafico e nelle etichette.
constexpr uint16_t PROBE_DISPLAY_COLORS[MAXIMUM_PROBE_COUNT] = { // Colori stabili per riconoscere ogni sonda nelle due viste.
    TFT_RED, TFT_GREEN, TFT_CYAN, TFT_YELLOW,
    TFT_MAGENTA, TFT_ORANGE, TFT_WHITE, TFT_PINK
};

// Posizione delle righe nella vista numerica
constexpr int16_t NUMERIC_VIEW_FIRST_ROW_X_PIXELS = 12; // Margine sinistro delle righe con le temperature, in pixel.
constexpr int16_t NUMERIC_VIEW_FIRST_ROW_Y_PIXELS = 36; // Coordinata verticale della prima riga, in pixel.
constexpr int16_t NUMERIC_VIEW_ROW_SPACING_PIXELS = 22; // Distanza verticale tra due righe consecutive, in pixel.
constexpr uint8_t NUMERIC_TEMPERATURE_TEXT_BUFFER_CAPACITY = 48; // Spazio in byte per comporre una riga di temperatura.

// Posizione e scala del grafico
constexpr int16_t TEMPERATURE_GRAPH_LEFT_X_PIXELS = 42; // Bordo sinistro dell'area del grafico, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_RIGHT_X_PIXELS = 310; // Bordo destro, dove viene disegnata la misura piu recente, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_TOP_Y_PIXELS = 42; // Bordo superiore dell'area del grafico, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS = 184; // Bordo inferiore dell'area del grafico, in pixel.
constexpr float TEMPERATURE_GRAPH_AXIS_MARGIN_CELSIUS = 1.0f; // Gradi aggiunti sopra e sotto i dati per non schiacciare le curve sui bordi.
constexpr int16_t TEMPERATURE_GRAPH_AXIS_LABEL_X_PIXELS = 0; // Margine sinistro delle etichette della scala in gradi Celsius, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_MINIMUM_LABEL_Y_OFFSET_PIXELS = 8; // Spostamento verso l'alto dell'etichetta minima, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_TIME_LABEL_Y_OFFSET_PIXELS = 3; // Spostamento sotto il grafico delle etichette temporali, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_NO_DATA_MESSAGE_X_PIXELS = TEMPERATURE_GRAPH_LEFT_X_PIXELS + 8; // Coordinata orizzontale del messaggio senza misure.
constexpr int16_t TEMPERATURE_GRAPH_NO_DATA_MESSAGE_Y_PIXELS = 100; // Coordinata verticale del messaggio senza misure, in pixel.
constexpr uint8_t TEMPERATURE_GRAPH_TIME_LABEL_BUFFER_CAPACITY = 16; // Spazio in byte per l'etichetta del tempo iniziale.
constexpr uint8_t TEMPERATURE_GRAPH_AXIS_LABEL_BUFFER_CAPACITY = 16; // Spazio in byte per un valore della scala in gradi Celsius.
constexpr char TEMPERATURE_GRAPH_OLDEST_SAMPLE_TIME_FORMAT[] = "-%u s"; // Forma dell'etichetta del campione piu vecchio, in secondi.
constexpr char TEMPERATURE_GRAPH_CURRENT_TIME_LABEL[] = "ora"; // Etichetta dell'istante della misura piu recente.
constexpr char TEMPERATURE_GRAPH_TEMPERATURE_LABEL_FORMAT[] = "%.0f C"; // Forma delle etichette della scala verticale.
constexpr char TEMPERATURE_GRAPH_NO_VALID_DATA_MESSAGE[] = "In attesa di misure valide"; // Messaggio mostrato prima della prima lettura valida.

// Posizione e forma della legenda del grafico
constexpr uint8_t TEMPERATURE_GRAPH_LEGEND_COLUMN_COUNT = 4; // Numero di sonde mostrate su ciascuna riga della legenda.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_COLUMN_WIDTH_PIXELS = 80; // Distanza orizzontale tra le colonne della legenda, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_FIRST_COLUMN_X_PIXELS = 0; // Coordinata orizzontale della prima colonna, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_FIRST_ROW_Y_PIXELS = 200; // Coordinata verticale della prima riga, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_ROW_SPACING_PIXELS = 10; // Distanza verticale tra le righe della legenda, in pixel.
constexpr uint8_t TEMPERATURE_GRAPH_LEGEND_TEXT_BUFFER_CAPACITY = 20; // Spazio in byte per il nome e il valore di una sonda.
constexpr char TEMPERATURE_GRAPH_VALID_PROBE_LABEL_FORMAT[] = "S%u %.1f"; // Forma della voce legenda quando la misura e valida.
constexpr char TEMPERATURE_GRAPH_UNAVAILABLE_PROBE_LABEL_FORMAT[] = "S%u --"; // Forma della voce legenda quando manca la misura.

// Testi e posizione dei messaggi della schermata
constexpr int16_t TEMPERATURE_SCREEN_TITLE_X_PIXELS = 8; // Coordinata orizzontale del titolo, in pixel.
constexpr int16_t TEMPERATURE_SCREEN_TITLE_Y_PIXELS = 8; // Coordinata verticale del titolo, in pixel.
constexpr int16_t TEMPERATURE_SCREEN_NO_PROBE_X_PIXELS = 8; // Coordinata orizzontale dell'avviso quando non ci sono sonde.
constexpr int16_t TEMPERATURE_SCREEN_NO_PROBE_TITLE_Y_PIXELS = 60; // Coordinata verticale della prima riga dell'avviso.
constexpr int16_t TEMPERATURE_SCREEN_NO_PROBE_HELP_Y_PIXELS = 84; // Coordinata verticale dell'istruzione di controllo.
constexpr int16_t TEMPERATURE_SCREEN_BUTTON_HELP_X_PIXELS = 8; // Coordinata orizzontale del suggerimento sul pulsante.
constexpr int16_t TEMPERATURE_SCREEN_BUTTON_HELP_Y_PIXELS = 230; // Coordinata verticale del suggerimento sul pulsante.
constexpr char TEMPERATURE_SCREEN_NUMERIC_TITLE[] = "Temperature sonde"; // Titolo mostrato nella vista numerica.
constexpr char TEMPERATURE_SCREEN_GRAPH_TITLE[] = "Andamento temperature"; // Titolo mostrato nella vista grafica.
constexpr char TEMPERATURE_SCREEN_NO_PROBE_MESSAGE[] = "Nessuna sonda DS18B20"; // Avviso quando all'avvio non viene trovata una sonda.
constexpr char TEMPERATURE_SCREEN_CHECK_PROBES_MESSAGE[] = "Verificare cavi e riavviare"; // Istruzione per ripristinare il rilevamento delle sonde.
constexpr char TEMPERATURE_SCREEN_BUTTON_HELP_MESSAGE[] = "BOOT: valori / grafico"; // Suggerimento per alternare le viste.
constexpr char NUMERIC_VIEW_VALID_TEMPERATURE_FORMAT[] = "S%u: %.2f C"; // Forma di una temperatura valida nella vista numerica.
constexpr char NUMERIC_VIEW_UNAVAILABLE_TEMPERATURE_FORMAT[] = "S%u: -- (non disponibile)"; // Forma di una lettura assente nella vista numerica.

#endif
