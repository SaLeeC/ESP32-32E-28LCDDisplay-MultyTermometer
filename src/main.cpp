/*
 * Scopo: misurare la temperatura di piu sonde DS18B20 e mostrarla sul display.
 *
 * Il programma separa il lavoro in passaggi facili da seguire:
 * - setup() prepara il display, il pulsante e il bus delle sonde;
 * - sampleTemperatures() avvia e legge le conversioni senza fermare il programma;
 * - pollButton() rileva una pressione stabile del pulsante BOOT;
 * - drawScreen() mostra i valori o il grafico richiesto.
 *
 * Le conversioni delle sonde richiedono tempo. Per questo il programma le avvia,
 * continua nel frattempo a controllare pulsante e display, e legge i risultati
 * solo quando sono trascorsi i 750 ms necessari alla risoluzione selezionata.
 */
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <TFT_eSPI.h>
#include <math.h>

constexpr uint8_t SENSOR_PIN = 27; // GPIO collegato al filo dati comune delle sonde DS18B20.
constexpr uint8_t BUTTON_PIN = 0; // GPIO del pulsante BOOT, letto come premuto quando vale LOW.
constexpr uint8_t MAX_PROBES = 8; // Numero massimo di sonde gestite contemporaneamente.
constexpr uint16_t HISTORY_SIZE = 120; // Campioni conservati per sonda; a un campione al secondo sono circa due minuti.
constexpr uint32_t SAMPLE_INTERVAL_MS = 1000; // Millisecondi tra l'avvio di due conversioni successive.
constexpr uint32_t CONVERSION_MS = 750; // Attesa in millisecondi per completare una conversione DS18B20 a 12 bit.
constexpr uint32_t DEBOUNCE_MS = 40; // Millisecondi di stabilita richiesti per ignorare il rimbalzo del pulsante.

// Raggruppa tutti i dati che appartengono a una singola sonda rilevata.
struct Probe {
    DeviceAddress address; // Indirizzo ROM univoco a 64 bit usato per interrogare sempre la stessa sonda.
    float temperature; // Ultima temperatura valida in gradi Celsius, oppure NAN se la lettura non e disponibile.
    float history[HISTORY_SIZE]; // Storico circolare delle ultime HISTORY_SIZE letture, in gradi Celsius.
};

OneWire bus(SENSOR_PIN); // Gestisce la comunicazione elettrica OneWire sul GPIO delle sonde.
DallasTemperature sensors(&bus); // Traduce le richieste di temperatura nei comandi per le sonde DS18B20.
TFT_eSPI display; // Interfaccia per disegnare testi, assi e grafici sul display TFT.
Probe probes[MAX_PROBES]; // Dati delle sonde riconosciute, ordinate come S1, S2 e cosi via.
// Assegna a ogni sonda un colore stabile, cosi il valore e la curva si riconoscono facilmente.
const uint16_t colors[MAX_PROBES] = {
    TFT_RED, TFT_GREEN, TFT_CYAN, TFT_YELLOW,
    TFT_MAGENTA, TFT_ORANGE, TFT_WHITE, TFT_PINK
};
uint8_t probeCount = 0; // Numero di sonde utilizzabili, che non supera MAX_PROBES.
uint16_t historyHead = 0; // Prossima posizione dello storico circolare in cui scrivere un campione.
uint16_t historyCount = 0; // Numero di campioni presenti nello storico, anche se alcune letture non sono valide.
uint32_t conversionStarted = 0; // Istante millis() in cui e stata avviata l'ultima conversione.
bool conversionPending = false; // Indica se una conversione e avviata e si sta ancora aspettando il risultato.
bool graphView = false; // Se false mostra i valori numerici; se true mostra il grafico.
bool redraw = true; // Richiede di ridisegnare lo schermo dopo un cambio di vista o una nuova misura.

/*
 * Disegna una riga numerica per ogni sonda riconosciuta.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: scrive sul display; mostra "--" quando la temperatura non e valida.
 */
void drawValues() {
    display.setTextFont(2);
    for (uint8_t i = 0; i < probeCount; ++i) { // Indice della sonda corrente nell'elenco da 0 a probeCount - 1.
        char text[48]; // Buffer temporaneo per comporre la riga senza usare allocazioni dinamiche.
        if (isfinite(probes[i].temperature)) {
            snprintf(text, sizeof(text), "S%u: %.2f C", i + 1,
                     probes[i].temperature);
        } else {
            // Una lettura non valida non viene trasformata in una temperatura fittizia.
            snprintf(text, sizeof(text), "S%u: -- (non disponibile)", i + 1);
        }
        display.setTextColor(colors[i], TFT_BLACK);
        display.drawString(text, 12, 36 + i * 22);
    }
}

/*
 * Disegna lo storico delle sonde come grafico temporale.
 * Prima trova l'intervallo delle temperature valide, poi colloca i campioni
 * sugli assi; un campione non valido interrompe la linea invece di collegare
 * tra loro due misure separate da un errore.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: legge gli storici e scrive assi, curve, etichette e legenda sul display.
 */
void drawGraph() {
    constexpr int LEFT = 42; // Coordinata orizzontale in pixel del bordo sinistro dell'area del grafico.
    constexpr int RIGHT = 310; // Coordinata orizzontale in pixel del bordo destro, dove arriva la misura piu recente.
    constexpr int TOP = 42; // Coordinata verticale in pixel del limite superiore del grafico.
    constexpr int BOTTOM = 184; // Coordinata verticale in pixel del limite inferiore del grafico.
    float low = INFINITY; // Minimo tra le temperature valide, usato per calcolare la scala verticale.
    float high = -INFINITY; // Massimo tra le temperature valide, usato per calcolare la scala verticale.

    // Cerca l'intervallo reale di temperature, ignorando i valori NAN che indicano una lettura assente.
    for (uint8_t p = 0; p < probeCount; ++p) { // Indice della sonda di cui si esamina lo storico.
        for (uint16_t i = 0; i < historyCount; ++i) { // Posizione dello storico gia riempita.
            float value = probes[p].history[i]; // Campione in gradi Celsius da confrontare con minimo e massimo.
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
        // Aggiunge un grado di margine e arrotonda i limiti: il grafico resta leggibile
        // e high resta maggiore di low anche se tutte le letture hanno lo stesso valore.
        low = floorf(low - 1.0f);
        high = ceilf(high + 1.0f);
        char label[16]; // Buffer temporaneo per le etichette numeriche della scala in gradi Celsius.
        snprintf(label, sizeof(label), "%.0f C", high);
        display.drawString(label, 0, TOP);
        snprintf(label, sizeof(label), "%.0f C", low);
        display.drawString(label, 0, BOTTOM - 8);

        for (uint8_t p = 0; p < probeCount; ++p) { // Disegna separatamente la curva di ciascuna sonda.
            bool previousValid = false; // Ricorda se esiste un punto valido da collegare al campione corrente.
            int previousX = 0; // Coordinata orizzontale in pixel del punto valido precedente.
            int previousY = 0; // Coordinata verticale in pixel del punto valido precedente.

            for (uint16_t i = 0; i < historyCount; ++i) { // Percorre i campioni dal piu vecchio al piu recente.
                // historyHead punta alla prossima cella da sovrascrivere. Questa formula ricostruisce
                // la posizione del campione piu vecchio e gestisce il ritorno circolare a indice zero.
                uint16_t index =
                    (historyHead + HISTORY_SIZE - historyCount + i) % HISTORY_SIZE; // Indice effettivo nel buffer circolare.
                float value = probes[p].history[index]; // Temperatura del campione corrente, oppure NAN se non valida.

                if (!isfinite(value)) {
                    // Non collegare attraverso una misura mancante: il vuoto segnala il problema nel grafico.
                    previousValid = false;
                    continue;
                }

                int x = RIGHT - (historyCount - 1 - i) *
                                (RIGHT - LEFT) / (HISTORY_SIZE - 1); // Posizione orizzontale: il campione piu recente e a destra.
                int y = BOTTOM - lroundf((value - low) * (BOTTOM - TOP) /
                                        (high - low)); // Posizione verticale: temperature maggiori sono piu vicine al bordo superiore.
                if (previousValid) {
                    display.drawLine(previousX, previousY, x, y, colors[p]);
                } else {
                    // Il primo punto valido dopo un errore viene mostrato, ma non collegato al tratto precedente.
                    display.drawPixel(x, y, colors[p]);
                }
                previousX = x;
                previousY = y;
                previousValid = true;
            }
        }
    } else {
        // Questa situazione si verifica all'avvio o se non e ancora arrivata alcuna misura valida.
        display.drawString("In attesa di misure valide", LEFT + 8, 100);
    }

    for (uint8_t p = 0; p < probeCount; ++p) { // Prepara una legenda compatta con il colore assegnato alla sonda.
        char label[20]; // Buffer per il numero della sonda e la sua temperatura piu recente.
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

/*
 * Ridisegna l'intera schermata dopo una richiesta di aggiornamento.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: cancella il display, sceglie la vista e azzera il flag redraw.
 */
void drawScreen() {
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextFont(2);
    display.drawString(graphView ? "Andamento temperature" : "Temperature sonde",
                       8, 8);
    if (probeCount == 0) {
        // Non ci sono dati da mostrare: fornisce un'indicazione pratica per il ripristino.
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
    redraw = false; // Lo schermo e aggiornato; un nuovo evento potra richiedere un altro ridisegno.
}

/*
 * Rileva una pressione stabile del pulsante BOOT e alterna le due viste.
 * Parametri: now, valore corrente di millis() in millisecondi.
 * Restituisce: nulla.
 * Effetti: aggiorna graphView e richiede il ridisegno quando il pulsante viene premuto.
 */
void pollButton(uint32_t now) {
    static bool lastRaw = HIGH; // Ultimo livello elettrico letto, conservato tra una chiamata e la successiva.
    static bool stable = HIGH; // Ultimo livello accettato dopo il controllo anti-rimbalzo.
    static uint32_t changedAt = 0; // Istante in cui il livello elettrico grezzo e cambiato per l'ultima volta.
    bool raw = digitalRead(BUTTON_PIN); // LOW significa premuto perche il pin usa INPUT_PULLUP.

    if (raw != lastRaw) {
        lastRaw = raw;
        changedAt = now; // Ricomincia l'attesa se il contatto continua a oscillare.
    }
    if (raw != stable && uint32_t(now - changedAt) >= DEBOUNCE_MS) {
        stable = raw;
        if (stable == LOW) {
            // Agisce solo alla pressione; il rilascio non provoca un secondo cambio di vista.
            graphView = !graphView;
            redraw = true;
        }
    }
}

/*
 * Completa le conversioni in corso e, quando e il momento, ne avvia una nuova.
 * Parametri: now, valore corrente di millis() in millisecondi.
 * Restituisce: nulla.
 * Effetti: aggiorna temperature, buffer circolari e flag usati dal disegno.
 *
 * La funzione non aspetta la conversione con un ritardo bloccante. Cosi il loop
 * puo continuare a controllare BOOT e aggiornare lo schermo mentre le sonde misurano.
 */
void sampleTemperatures(uint32_t now) {
    if (probeCount == 0) {
        return;
    }

    // Se la conversione avviata in precedenza e terminata, legge tutte le sonde.
    if (conversionPending &&
        uint32_t(now - conversionStarted) >= CONVERSION_MS) {
        for (uint8_t i = 0; i < probeCount; ++i) { // Legge la sonda usando il suo indirizzo ROM, non solo il numero nell'elenco.
            float value = sensors.getTempC(probes[i].address); // Risultato grezzo in gradi Celsius restituito dalla libreria.
            if (!isfinite(value) || value == DEVICE_DISCONNECTED_C ||
                value < -55.0f || value > 125.0f) {
                value = NAN; // NAN rende esplicito che la misura non puo essere usata ne mostrata come temperatura.
            }
            probes[i].temperature = value;
            probes[i].history[historyHead] = value; // Tutte le sonde condividono la stessa posizione del campione temporale.
        }

        // Avanza una sola volta dopo aver salvato tutte le sonde, cosi i loro campioni restano allineati.
        historyHead = (historyHead + 1) % HISTORY_SIZE;
        if (historyCount < HISTORY_SIZE) {
            ++historyCount; // Prima del riempimento, indica quante posizioni contengono gia un campione.
        }
        conversionPending = false;
        redraw = true;
    }

    // Avvia la misura successiva solo quando non c'e gia una conversione in corso.
    if (!conversionPending &&
        uint32_t(now - conversionStarted) >= SAMPLE_INTERVAL_MS) {
        sensors.requestTemperatures();
        conversionStarted = now; // Da questo istante decorrono i 750 ms di conversione.
        conversionPending = true;
    }
}

/*
 * Configura le periferiche e trova le sonde presenti all'avvio.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: inizializza seriale, pulsante, display e bus OneWire; prepara
 * gli storici e avvia la prima conversione senza attendere in modo bloccante.
 */
void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Usa la resistenza interna: il pulsante premuto porta GPIO0 a LOW.
    display.init();
    display.setRotation(1); // Orienta il display in orizzontale per adattarlo alla grafica 320 x 240.
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Accende la retroilluminazione configurata in platformio.ini.

    sensors.begin();
    DeviceAddress address; // Buffer temporaneo in cui la libreria copia l'indirizzo della sonda trovata.
    uint8_t found = 0; // Conta tutte le DS18B20 trovate, incluse quelle oltre il limite gestibile.

    // La ricerca avviene solo all'avvio: dopo aver aggiunto o rimosso sonde occorre riavviare.
    for (uint8_t i = 0; i < sensors.getDeviceCount(); ++i) { // Indice assegnato dalla libreria ai dispositivi presenti sul bus.
        if (!sensors.getAddress(address, i) || address[0] != 0x28) {
            continue; // Salta dispositivi non leggibili o che non appartengono alla famiglia DS18B20.
        }
        ++found;
        if (probeCount == MAX_PROBES) {
            continue; // Conta la sonda ma non la memorizza se e stato raggiunto il limite del firmware.
        }
        Probe &probe = probes[probeCount]; // Riferimento allo spazio che diventera la prossima sonda S1, S2, ecc.
        memcpy(probe.address, address, sizeof(DeviceAddress));
        probe.temperature = NAN;
        for (float &value : probe.history) { // Inizializza ogni campione come assente, evitando valori casuali in RAM.
            value = NAN;
        }

        // Stampa l'indirizzo ROM per poter associare in modo verificabile S1, S2, ecc. alle sonde fisiche.
        Serial.printf("S%u ROM:", probeCount + 1);
        for (uint8_t byte : probe.address) { // Mostra tutti i byte dell'indirizzo univoco in formato esadecimale.
            Serial.printf(" %02X", byte);
        }
        Serial.println();
        ++probeCount;
    }
    if (found > MAX_PROBES) {
        // Le sonde oltre il limite sono ignorate, ma l'utente riceve un avviso sul monitor seriale.
        Serial.printf("Limite %u sonde: %u sonde ignorate\n",
                      MAX_PROBES, found - MAX_PROBES);
    }
    sensors.setResolution(12); // Usa la massima risoluzione DS18B20: a 12 bit servono fino a 750 ms.
    sensors.setWaitForConversion(false); // Lascia che il loop continui mentre la conversione avviene sul bus.
    if (probeCount > 0) {
        sensors.requestTemperatures();
        conversionStarted = millis();
        conversionPending = true;
    }
    drawScreen(); // Mostra subito la schermata iniziale, anche se la prima misura non e ancora pronta.
}

/*
 * Esegue ripetutamente il lavoro del firmware senza arrestare il microcontrollore.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: controlla il pulsante, aggiorna le misure e ridisegna solo quando serve.
 */
void loop() {
    uint32_t now = millis(); // Un unico riferimento temporale per le operazioni di questo passaggio del loop.
    pollButton(now);
    sampleTemperatures(now);
    if (redraw) {
        drawScreen();
    }
}
