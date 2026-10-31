/*
 * Modulo del display TFT e delle due viste disponibili.
 *
 * La vista numerica rende immediato il controllo delle letture correnti.
 * La vista grafica usa uno storico circolare: il campione piu recente appare
 * a destra e una lettura non valida interrompe la curva, invece di nascondere
 * il problema collegando punti che non sono stati realmente misurati.
 */

void initializeTemperatureDisplay() {
    temperatureDisplay.init();
    temperatureDisplay.setRotation(TEMPERATURE_DISPLAY_LANDSCAPE_ROTATION); // Usa l'orientamento orizzontale previsto per la scheda.
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Accende la retroilluminazione configurata in platformio.ini.
}

void drawCurrentTemperatureScreen() {
    temperatureDisplay.fillScreen(TFT_BLACK); // Pulisce la schermata prima di disegnare la nuova vista.
    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.drawString(
        graphViewIsActive ? TEMPERATURE_SCREEN_GRAPH_TITLE : TEMPERATURE_SCREEN_NUMERIC_TITLE,
        TEMPERATURE_SCREEN_TITLE_X_PIXELS,
        TEMPERATURE_SCREEN_TITLE_Y_PIXELS);

    if (activeTemperatureProbeCount == 0) {
        // Senza sonde non esistono misure da presentare: mostra come controllare il cablaggio.
        temperatureDisplay.drawString(
            TEMPERATURE_SCREEN_NO_PROBE_MESSAGE,
            TEMPERATURE_SCREEN_NO_PROBE_X_PIXELS,
            TEMPERATURE_SCREEN_NO_PROBE_TITLE_Y_PIXELS);
        temperatureDisplay.drawString(
            TEMPERATURE_SCREEN_CHECK_PROBES_MESSAGE,
            TEMPERATURE_SCREEN_NO_PROBE_X_PIXELS,
            TEMPERATURE_SCREEN_NO_PROBE_HELP_Y_PIXELS);
    } else if (graphViewIsActive) {
        drawTemperatureHistoryGraph();
    } else {
        drawNumericTemperatureValues();
    }

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    temperatureDisplay.drawString(
        TEMPERATURE_SCREEN_BUTTON_HELP_MESSAGE,
        TEMPERATURE_SCREEN_BUTTON_HELP_X_PIXELS,
        TEMPERATURE_SCREEN_BUTTON_HELP_Y_PIXELS);
    temperatureDisplayNeedsRedraw = false; // La schermata e aggiornata; un nuovo evento potra richiedere un altro disegno.
}

void drawNumericTemperatureValues() {
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);

    for (uint8_t probeIndex = 0; probeIndex < activeTemperatureProbeCount; ++probeIndex) { // Disegna una riga per ogni sonda riconosciuta.
        char temperatureText[NUMERIC_TEMPERATURE_TEXT_BUFFER_CAPACITY]; // Buffer temporaneo per comporre la riga senza allocazioni dinamiche.
        const unsigned int displayedProbeNumber = static_cast<unsigned int>(probeIndex + 1); // Numero visibile della sonda, a partire da S1.

        if (isfinite(temperatureProbes[probeIndex].latestTemperatureCelsius)) {
            snprintf(temperatureText,
                     sizeof(temperatureText),
                     NUMERIC_VIEW_VALID_TEMPERATURE_FORMAT,
                     displayedProbeNumber,
                     temperatureProbes[probeIndex].latestTemperatureCelsius);
        } else {
            // Una lettura assente non viene trasformata in una temperatura fittizia.
            snprintf(temperatureText,
                     sizeof(temperatureText),
                     NUMERIC_VIEW_UNAVAILABLE_TEMPERATURE_FORMAT,
                     displayedProbeNumber);
        }

        temperatureDisplay.setTextColor(PROBE_DISPLAY_COLORS[probeIndex], TFT_BLACK);
        temperatureDisplay.drawString(
            temperatureText,
            NUMERIC_VIEW_FIRST_ROW_X_PIXELS,
            NUMERIC_VIEW_FIRST_ROW_Y_PIXELS + probeIndex * NUMERIC_VIEW_ROW_SPACING_PIXELS);
    }
}

bool findValidTemperatureHistoryRange(float& lowestValidTemperatureCelsius,
                                      float& highestValidTemperatureCelsius) {
    lowestValidTemperatureCelsius = INFINITY; // Finche non si trova una misura valida, non esiste ancora un minimo.
    highestValidTemperatureCelsius = -INFINITY; // Finche non si trova una misura valida, non esiste ancora un massimo.

    // Cerca l'intervallo reale di temperature, ignorando NAN e altri valori non numerici.
    for (uint8_t probeIndex = 0; probeIndex < activeTemperatureProbeCount; ++probeIndex) { // Esamina lo storico di ciascuna sonda.
        for (uint16_t sampleOffset = 0; sampleOffset < storedTemperatureHistorySampleCount; ++sampleOffset) { // Esamina i campioni gia registrati.
            const float historyTemperatureCelsius =
                temperatureProbes[probeIndex].temperatureHistoryCelsius[sampleOffset]; // Campione in gradi Celsius da valutare.

            if (isfinite(historyTemperatureCelsius)) {
                lowestValidTemperatureCelsius =
                    fminf(lowestValidTemperatureCelsius, historyTemperatureCelsius);
                highestValidTemperatureCelsius =
                    fmaxf(highestValidTemperatureCelsius, historyTemperatureCelsius);
            }
        }
    }

    return isfinite(lowestValidTemperatureCelsius); // False significa che non e ancora arrivata alcuna misura valida.
}

void drawTemperatureGraphAxes() {
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.drawFastVLine(
        TEMPERATURE_GRAPH_LEFT_X_PIXELS,
        TEMPERATURE_GRAPH_TOP_Y_PIXELS,
        TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS - TEMPERATURE_GRAPH_TOP_Y_PIXELS + 1,
        TFT_DARKGREY);
    temperatureDisplay.drawFastHLine(
        TEMPERATURE_GRAPH_LEFT_X_PIXELS,
        TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS,
        TEMPERATURE_GRAPH_RIGHT_X_PIXELS - TEMPERATURE_GRAPH_LEFT_X_PIXELS + 1,
        TFT_DARKGREY);

    char oldestSampleTimeLabel[TEMPERATURE_GRAPH_TIME_LABEL_BUFFER_CAPACITY]; // Buffer per indicare il tempo del campione piu vecchio.
    snprintf(oldestSampleTimeLabel,
             sizeof(oldestSampleTimeLabel),
             TEMPERATURE_GRAPH_OLDEST_SAMPLE_TIME_FORMAT,
             static_cast<unsigned int>(TEMPERATURE_HISTORY_CAPACITY - 1));
    temperatureDisplay.drawString(
        oldestSampleTimeLabel,
        TEMPERATURE_GRAPH_LEFT_X_PIXELS,
        TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS + TEMPERATURE_GRAPH_TIME_LABEL_Y_OFFSET_PIXELS);
    temperatureDisplay.drawRightString(
        TEMPERATURE_GRAPH_CURRENT_TIME_LABEL,
        TEMPERATURE_GRAPH_RIGHT_X_PIXELS,
        TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS + TEMPERATURE_GRAPH_TIME_LABEL_Y_OFFSET_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

void drawTemperatureGraphTemperatureAxisLabels(float lowestAxisTemperatureCelsius,
                                               float highestAxisTemperatureCelsius) {
    char temperatureAxisLabel[TEMPERATURE_GRAPH_AXIS_LABEL_BUFFER_CAPACITY]; // Buffer riutilizzato per le etichette della scala verticale.

    snprintf(temperatureAxisLabel,
             sizeof(temperatureAxisLabel),
             TEMPERATURE_GRAPH_TEMPERATURE_LABEL_FORMAT,
             highestAxisTemperatureCelsius);
    temperatureDisplay.drawString(
        temperatureAxisLabel,
        TEMPERATURE_GRAPH_AXIS_LABEL_X_PIXELS,
        TEMPERATURE_GRAPH_TOP_Y_PIXELS);

    snprintf(temperatureAxisLabel,
             sizeof(temperatureAxisLabel),
             TEMPERATURE_GRAPH_TEMPERATURE_LABEL_FORMAT,
             lowestAxisTemperatureCelsius);
    temperatureDisplay.drawString(
        temperatureAxisLabel,
        TEMPERATURE_GRAPH_AXIS_LABEL_X_PIXELS,
        TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS - TEMPERATURE_GRAPH_MINIMUM_LABEL_Y_OFFSET_PIXELS);
}

uint16_t calculateChronologicalHistoryBufferIndex(uint16_t chronologicalSampleOffset) {
    // nextTemperatureHistoryWriteIndex indica la prossima cella da sovrascrivere.
    // Sottraendo quanti campioni sono presenti si trova il piu vecchio, poi si avanza
    // dell'indice richiesto e si torna a zero quando si supera la fine del buffer.
    return (nextTemperatureHistoryWriteIndex + TEMPERATURE_HISTORY_CAPACITY -
            storedTemperatureHistorySampleCount + chronologicalSampleOffset) %
           TEMPERATURE_HISTORY_CAPACITY;
}

void drawTemperatureHistoryForProbe(uint8_t probeIndex,
                                    float lowestAxisTemperatureCelsius,
                                    float highestAxisTemperatureCelsius) {
    bool previousHistorySampleIsValid = false; // Indica se esiste un punto valido da collegare al campione attuale.
    int16_t previousSampleXPositionPixels = 0; // Coordinata orizzontale in pixel del punto valido precedente.
    int16_t previousSampleYPositionPixels = 0; // Coordinata verticale in pixel del punto valido precedente.

    for (uint16_t chronologicalSampleOffset = 0;
         chronologicalSampleOffset < storedTemperatureHistorySampleCount;
         ++chronologicalSampleOffset) { // Percorre i campioni dal piu vecchio al piu recente.
        const uint16_t historyBufferIndex =
            calculateChronologicalHistoryBufferIndex(chronologicalSampleOffset); // Indice effettivo nel buffer circolare.
        const float historyTemperatureCelsius =
            temperatureProbes[probeIndex].temperatureHistoryCelsius[historyBufferIndex]; // Temperatura del campione, oppure NAN.

        if (!isfinite(historyTemperatureCelsius)) {
            // Un dato assente interrompe la linea: non si suggerisce una misura che non e avvenuta.
            previousHistorySampleIsValid = false;
            continue;
        }

        const uint16_t newerSampleCount =
            storedTemperatureHistorySampleCount - 1 - chronologicalSampleOffset; // Quanti campioni piu recenti separano questo punto dall'ultimo.
        const int16_t horizontalDistanceFromLatestSamplePixels =
            static_cast<int16_t>(
                newerSampleCount *
                (TEMPERATURE_GRAPH_RIGHT_X_PIXELS - TEMPERATURE_GRAPH_LEFT_X_PIXELS) /
                (TEMPERATURE_HISTORY_CAPACITY - 1)); // Distanza orizzontale proporzionale al tempo trascorso.
        const int16_t sampleXPositionPixels =
            TEMPERATURE_GRAPH_RIGHT_X_PIXELS - horizontalDistanceFromLatestSamplePixels; // La lettura piu recente resta a destra.
        const float verticalTemperatureRangeCelsius =
            highestAxisTemperatureCelsius - lowestAxisTemperatureCelsius; // Intervallo usato per convertire gradi in pixel.
        const int16_t sampleHeightAboveBottomPixels =
            static_cast<int16_t>(lroundf(
                (historyTemperatureCelsius - lowestAxisTemperatureCelsius) *
                (TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS - TEMPERATURE_GRAPH_TOP_Y_PIXELS) /
                verticalTemperatureRangeCelsius)); // Altezza proporzionale alla temperatura, in pixel.
        const int16_t sampleYPositionPixels =
            TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS - sampleHeightAboveBottomPixels; // Temperature maggiori sono piu vicine al bordo superiore.

        if (previousHistorySampleIsValid) {
            temperatureDisplay.drawLine(
                previousSampleXPositionPixels,
                previousSampleYPositionPixels,
                sampleXPositionPixels,
                sampleYPositionPixels,
                PROBE_DISPLAY_COLORS[probeIndex]);
        } else {
            // Il primo punto dopo un'interruzione si vede, ma non viene collegato al tratto precedente.
            temperatureDisplay.drawPixel(
                sampleXPositionPixels,
                sampleYPositionPixels,
                PROBE_DISPLAY_COLORS[probeIndex]);
        }

        previousSampleXPositionPixels = sampleXPositionPixels;
        previousSampleYPositionPixels = sampleYPositionPixels;
        previousHistorySampleIsValid = true;
    }
}

void drawTemperatureGraphLegend() {
    for (uint8_t probeIndex = 0; probeIndex < activeTemperatureProbeCount; ++probeIndex) { // Disegna una voce colorata per ciascuna sonda.
        char probeLegendText[TEMPERATURE_GRAPH_LEGEND_TEXT_BUFFER_CAPACITY]; // Buffer per numero della sonda e ultima lettura.
        const unsigned int displayedProbeNumber = static_cast<unsigned int>(probeIndex + 1); // Numero visibile della sonda, a partire da S1.

        if (isfinite(temperatureProbes[probeIndex].latestTemperatureCelsius)) {
            snprintf(probeLegendText,
                     sizeof(probeLegendText),
                     TEMPERATURE_GRAPH_VALID_PROBE_LABEL_FORMAT,
                     displayedProbeNumber,
                     temperatureProbes[probeIndex].latestTemperatureCelsius);
        } else {
            snprintf(probeLegendText,
                     sizeof(probeLegendText),
                     TEMPERATURE_GRAPH_UNAVAILABLE_PROBE_LABEL_FORMAT,
                     displayedProbeNumber);
        }

        const int16_t legendColumnXPositionPixels =
            TEMPERATURE_GRAPH_LEGEND_FIRST_COLUMN_X_PIXELS +
            (probeIndex % TEMPERATURE_GRAPH_LEGEND_COLUMN_COUNT) *
                TEMPERATURE_GRAPH_LEGEND_COLUMN_WIDTH_PIXELS; // Colloca le prime quattro sonde sulla prima riga.
        const int16_t legendRowYPositionPixels =
            TEMPERATURE_GRAPH_LEGEND_FIRST_ROW_Y_PIXELS +
            (probeIndex / TEMPERATURE_GRAPH_LEGEND_COLUMN_COUNT) *
                TEMPERATURE_GRAPH_LEGEND_ROW_SPACING_PIXELS; // Le sonde successive passano alla riga inferiore.

        temperatureDisplay.setTextColor(PROBE_DISPLAY_COLORS[probeIndex], TFT_BLACK);
        temperatureDisplay.drawString(
            probeLegendText,
            legendColumnXPositionPixels,
            legendRowYPositionPixels);
    }
}

void drawTemperatureHistoryGraph() {
    drawTemperatureGraphAxes(); // Gli assi temporali sono utili anche quando non sono ancora arrivate misure valide.

    float lowestValidTemperatureCelsius; // Minimo reale usato per costruire la scala verticale.
    float highestValidTemperatureCelsius; // Massimo reale usato per costruire la scala verticale.
    if (findValidTemperatureHistoryRange(
            lowestValidTemperatureCelsius,
            highestValidTemperatureCelsius)) {
        // Aggiunge un grado di margine e arrotonda i limiti: la curva non tocca i bordi
        // e la scala resta valida anche se tutte le letture hanno lo stesso valore.
        const float lowestAxisTemperatureCelsius =
            floorf(lowestValidTemperatureCelsius - TEMPERATURE_GRAPH_AXIS_MARGIN_CELSIUS);
        const float highestAxisTemperatureCelsius =
            ceilf(highestValidTemperatureCelsius + TEMPERATURE_GRAPH_AXIS_MARGIN_CELSIUS);

        drawTemperatureGraphTemperatureAxisLabels(
            lowestAxisTemperatureCelsius,
            highestAxisTemperatureCelsius);
        for (uint8_t probeIndex = 0; probeIndex < activeTemperatureProbeCount; ++probeIndex) { // Disegna una curva indipendente per ogni sonda.
            drawTemperatureHistoryForProbe(
                probeIndex,
                lowestAxisTemperatureCelsius,
                highestAxisTemperatureCelsius);
        }
    } else {
        // All'avvio lo storico e vuoto; si attende senza inventare una temperatura o una scala.
        temperatureDisplay.drawString(
            TEMPERATURE_GRAPH_NO_VALID_DATA_MESSAGE,
            TEMPERATURE_GRAPH_NO_DATA_MESSAGE_X_PIXELS,
            TEMPERATURE_GRAPH_NO_DATA_MESSAGE_Y_PIXELS);
    }

    drawTemperatureGraphLegend(); // La legenda mostra sempre anche le ultime letture.
}
