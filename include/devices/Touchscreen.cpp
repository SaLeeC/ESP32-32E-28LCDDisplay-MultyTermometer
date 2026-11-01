/*
 * Modulo del touchscreen resistivo XPT2046.
 *
 * La libreria TFT_Touch comunica con il controller tramite linee GPIO dedicate,
 * separate da quelle del display. All'avvio una breve presentazione introduce
 * il prodotto e tre bersagli in posizioni distinte permettono di acquisire una
 * calibrazione temporanea. I colori confermano rilevamento e acquisizione;
 * quando un punto fallisce si ripete solo quello, mentre BOOT puo escludere
 * il touch durante tutta la procedura.
 *
 * La calibrazione non viene salvata: viene ricalcolata a ogni accensione.
 * Se il controller non risponde, BOOT consente di proseguire e cambiare vista
 * con il pulsante fisico, senza usare coordinate touch non verificate.
 */

namespace {

// Un punto grezzo e la media delle letture ricevute durante un singolo tocco.
struct RawTouchCalibrationPoint {
    int16_t rawX; // Coordinata X restituita dal convertitore XPT2046, prima della mappatura in pixel.
    int16_t rawY; // Coordinata Y restituita dal convertitore XPT2046, prima della mappatura in pixel.
};

// Esito possibile mentre si aspetta che l'operatore tocchi un bersaglio.
enum class TouchTargetCaptureResult : uint8_t {
    Captured, // Il punto e stato letto e puo essere usato per la calibrazione.
    TimedOut, // L'operatore non ha toccato il bersaglio entro il tempo previsto.
    BootButtonPressed, // L'operatore ha scelto di proseguire senza touch.
    NoUsableSamples // Il tocco e stato rilevato, ma non ha prodotto coordinate utilizzabili.
};

// Scelta dell'operatore dopo un messaggio di errore o un tempo scaduto.
enum class TouchRecoveryChoice : uint8_t {
    RetryCurrentTarget, // Un tocco ripresenta il solo bersaglio che non e stato acquisito.
    RetryAllTargets, // Un tocco ripete tutti i bersagli dopo una calibrazione geometrica non valida.
    ContinueWithBootButton // BOOT esclude il touch per questa accensione.
};

/*
 * Verifica il segnale elettrico di pressione e chiede alla libreria di
 * confermare una lettura stabile. Quando restituisce true, RawX() e RawY()
 * contengono le coordinate grezze dell'ultimo campione valido.
 */
bool resistiveTouchPanelIsPressed() {
    return digitalRead(XPT2046_TOUCH_INTERRUPT_PIN) == LOW &&
           resistiveTouchController.Pressed();
}

/*
 * Attende che venga rilasciato BOOT dopo averlo scelto come comando di
 * recupero, cosi la stessa pressione non puo cambiare subito la vista.
 */
void waitForBootButtonRelease() {
    while (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        waitWhileServicingMeasurements(STARTUP_TOUCH_WAIT_POLL_INTERVAL_MILLISECONDS);
    }
}

/*
 * Attende che venga sollevato il dito. Se il pannello resta rilevato come
 * premuto, BOOT interrompe l'attesa e permette comunque di proseguire.
 */
bool waitForTouchReleaseOrBootButton() {
    while (digitalRead(XPT2046_TOUCH_INTERRUPT_PIN) == LOW) {
        if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            waitForBootButtonRelease();
            return false; // false indica che BOOT e stato usato per uscire dall'attesa.
        }

        waitWhileServicingMeasurements(STARTUP_TOUCH_WAIT_POLL_INTERVAL_MILLISECONDS);
    }

    return true; // true indica che il dito e stato sollevato normalmente.
}

/*
 * Aspetta un tocco sul bersaglio e media le coordinate grezze mentre il dito
 * resta appoggiato. La pressione deve essere distinta da un rilascio prima
 * di poter acquisire il punto seguente.
 */
TouchTargetCaptureResult captureTouchCalibrationTarget(
    uint8_t calibrationTargetIndex,
    RawTouchCalibrationPoint& capturedRawPoint) {
    drawTouchCalibrationTarget(calibrationTargetIndex);
    const uint32_t touchWaitStartMilliseconds = millis(); // Istante da cui si misura il limite di attesa.

    while (!resistiveTouchPanelIsPressed()) {
        if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            waitForBootButtonRelease();
            return TouchTargetCaptureResult::BootButtonPressed; // BOOT e il recupero se il touch non risponde.
        }

        if (static_cast<uint32_t>(millis() - touchWaitStartMilliseconds) >=
            STARTUP_TOUCH_TARGET_TIMEOUT_MILLISECONDS) {
            return TouchTargetCaptureResult::TimedOut; // Non lascia la schermata di avvio in attesa senza spiegazioni.
        }

        waitWhileServicingMeasurements(STARTUP_TOUCH_WAIT_POLL_INTERVAL_MILLISECONDS);
    }

    drawTouchCalibrationTargetFeedback(
        calibrationTargetIndex,
        STARTUP_TOUCH_TARGET_DETECTED_INSTRUCTION,
        STARTUP_TOUCH_TARGET_DETECTED_COLOR,
        false);

    uint32_t accumulatedRawX = 0; // Somma usata per attenuare il rumore della lettura grezza X.
    uint32_t accumulatedRawY = 0; // Somma usata per attenuare il rumore della lettura grezza Y.
    uint8_t usableTouchSampleCount = 0; // Numero di campioni validi inclusi nelle due medie.
    uint8_t rejectedTouchSampleCount = 0; // Campioni letti ma fuori dai limiti plausibili del convertitore.
    int16_t lastRawTouchX = 0; // Ultima coordinata X letta, utile per diagnosticare un punto non acquisito.
    int16_t lastRawTouchY = 0; // Ultima coordinata Y letta, utile per diagnosticare un punto non acquisito.

    while (usableTouchSampleCount < STARTUP_TOUCH_CALIBRATION_RAW_SAMPLE_COUNT &&
           resistiveTouchPanelIsPressed()) {
        const int16_t currentRawTouchX =
            static_cast<int16_t>(resistiveTouchController.RawX()); // Coordinata grezza X fornita dalla libreria TFT_Touch.
        const int16_t currentRawTouchY =
            static_cast<int16_t>(resistiveTouchController.RawY()); // Coordinata grezza Y fornita dalla libreria TFT_Touch.
        lastRawTouchX = currentRawTouchX;
        lastRawTouchY = currentRawTouchY;

        if (currentRawTouchX >= XPT2046_MINIMUM_ACCEPTED_RAW_COORDINATE &&
            currentRawTouchX <= XPT2046_MAXIMUM_ACCEPTED_RAW_COORDINATE &&
            currentRawTouchY >= XPT2046_MINIMUM_ACCEPTED_RAW_COORDINATE &&
            currentRawTouchY <= XPT2046_MAXIMUM_ACCEPTED_RAW_COORDINATE) {
            accumulatedRawX += static_cast<uint16_t>(currentRawTouchX);
            accumulatedRawY += static_cast<uint16_t>(currentRawTouchY);
            ++usableTouchSampleCount;
        } else {
            ++rejectedTouchSampleCount;
        }

        waitWhileServicingMeasurements(STARTUP_TOUCH_CALIBRATION_SAMPLE_INTERVAL_MILLISECONDS);
    }

    if (usableTouchSampleCount == 0) {
        Serial.printf(
            "Touch: punto %u/%u senza campioni validi; scartati %u, ultimo raw X=%d Y=%d.\n",
            static_cast<unsigned int>(calibrationTargetIndex + 1),
            static_cast<unsigned int>(STARTUP_TOUCH_CALIBRATION_TARGET_COUNT),
            static_cast<unsigned int>(rejectedTouchSampleCount),
            static_cast<int>(lastRawTouchX),
            static_cast<int>(lastRawTouchY));
        drawTouchCalibrationTargetFeedback(
            calibrationTargetIndex,
            STARTUP_TOUCH_TARGET_INVALID_DETAIL,
            STARTUP_TOUCH_TARGET_FAILURE_COLOR,
            false);
    } else {
        capturedRawPoint.rawX = static_cast<int16_t>(
            lroundf(static_cast<float>(accumulatedRawX) / usableTouchSampleCount));
        capturedRawPoint.rawY = static_cast<int16_t>(
            lroundf(static_cast<float>(accumulatedRawY) / usableTouchSampleCount));
        drawTouchCalibrationTargetFeedback(
            calibrationTargetIndex,
            STARTUP_TOUCH_TARGET_CAPTURED_INSTRUCTION,
            STARTUP_TOUCH_TARGET_CAPTURED_COLOR,
            true);
    }

    // Il feedback resta visibile mentre si attende il rilascio; BOOT permette sempre di saltare il touch.
    if (!waitForTouchReleaseOrBootButton()) {
        return TouchTargetCaptureResult::BootButtonPressed;
    }

    if (usableTouchSampleCount == 0) {
        return TouchTargetCaptureResult::NoUsableSamples;
    }

    Serial.printf(
        "Touch: punto %u/%u acquisito con %u campioni validi; X=%d, Y=%d.\n",
        static_cast<unsigned int>(calibrationTargetIndex + 1),
        static_cast<unsigned int>(STARTUP_TOUCH_CALIBRATION_TARGET_COUNT),
        static_cast<unsigned int>(usableTouchSampleCount),
        static_cast<int>(capturedRawPoint.rawX),
        static_cast<int>(capturedRawPoint.rawY));
    waitWhileServicingMeasurements(STARTUP_TOUCH_CAPTURE_FEEDBACK_DURATION_MILLISECONDS);
    return TouchTargetCaptureResult::Captured;
}

/*
 * Attende un tocco per avviare il tentativo indicato oppure BOOT per saltare
 * il touch. Il messaggio con le due scelte viene disegnato prima dell'attesa.
 */
TouchRecoveryChoice waitForTouchRetryOrBootButton(
    TouchRecoveryChoice touchRetryChoice) {
    while (true) {
        if (resistiveTouchPanelIsPressed()) {
            // Il rilascio conferma il retry; BOOT resta disponibile anche se il pannello non lo rileva.
            if (waitForTouchReleaseOrBootButton()) {
                return touchRetryChoice;
            }
            return TouchRecoveryChoice::ContinueWithBootButton;
        }

        if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            waitForBootButtonRelease(); // Evita che la stessa pressione venga interpretata due volte nel loop normale.
            return TouchRecoveryChoice::ContinueWithBootButton;
        }

        waitWhileServicingMeasurements(STARTUP_TOUCH_WAIT_POLL_INTERVAL_MILLISECONDS);
    }
}

/*
 * Calcola una trasformazione affine da tre punti non allineati.
 * Una trasformazione affine corregge anche scambio, inclinazione e inversione
 * degli assi, non soltanto il minimo e massimo di X e Y.
 */
bool calculateTouchScreenCalibration(
    const RawTouchCalibrationPoint rawCalibrationPoints[STARTUP_TOUCH_CALIBRATION_TARGET_COUNT],
    TouchScreenCalibrationTransform& calculatedCalibration) {
    for (uint8_t calibrationPointIndex = 0;
         calibrationPointIndex < STARTUP_TOUCH_CALIBRATION_TARGET_COUNT;
         ++calibrationPointIndex) {
        if (rawCalibrationPoints[calibrationPointIndex].rawX < XPT2046_MINIMUM_ACCEPTED_RAW_COORDINATE ||
            rawCalibrationPoints[calibrationPointIndex].rawX > XPT2046_MAXIMUM_ACCEPTED_RAW_COORDINATE ||
            rawCalibrationPoints[calibrationPointIndex].rawY < XPT2046_MINIMUM_ACCEPTED_RAW_COORDINATE ||
            rawCalibrationPoints[calibrationPointIndex].rawY > XPT2046_MAXIMUM_ACCEPTED_RAW_COORDINATE) {
            return false; // Un valore ai limiti del convertitore suggerisce un tocco o un collegamento non affidabile.
        }
    }

    const float rawFirstPointX = rawCalibrationPoints[0].rawX;
    const float rawFirstPointY = rawCalibrationPoints[0].rawY;
    const float rawSecondPointX = rawCalibrationPoints[1].rawX;
    const float rawSecondPointY = rawCalibrationPoints[1].rawY;
    const float rawThirdPointX = rawCalibrationPoints[2].rawX;
    const float rawThirdPointY = rawCalibrationPoints[2].rawY;
    const float rawFirstToSecondDeltaX = rawSecondPointX - rawFirstPointX;
    const float rawFirstToSecondDeltaY = rawSecondPointY - rawFirstPointY;
    const float rawFirstToThirdDeltaX = rawThirdPointX - rawFirstPointX;
    const float rawFirstToThirdDeltaY = rawThirdPointY - rawFirstPointY;
    const float rawCalibrationTriangleAreaTwice =
        rawFirstToSecondDeltaX * rawFirstToThirdDeltaY -
        rawFirstToThirdDeltaX * rawFirstToSecondDeltaY;

    if (!isfinite(rawCalibrationTriangleAreaTwice) ||
        fabsf(rawCalibrationTriangleAreaTwice) < TOUCH_CALIBRATION_MINIMUM_RAW_TRIANGLE_AREA) {
        return false; // Tre punti troppo vicini o allineati non definiscono una calibrazione stabile.
    }

    const float screenFirstPointX = STARTUP_TOUCH_CALIBRATION_TARGET_X_PIXELS[0];
    const float screenFirstPointY = STARTUP_TOUCH_CALIBRATION_TARGET_Y_PIXELS[0];
    const float screenFirstToSecondDeltaX =
        STARTUP_TOUCH_CALIBRATION_TARGET_X_PIXELS[1] - screenFirstPointX;
    const float screenFirstToSecondDeltaY =
        STARTUP_TOUCH_CALIBRATION_TARGET_Y_PIXELS[1] - screenFirstPointY;
    const float screenFirstToThirdDeltaX =
        STARTUP_TOUCH_CALIBRATION_TARGET_X_PIXELS[2] - screenFirstPointX;
    const float screenFirstToThirdDeltaY =
        STARTUP_TOUCH_CALIBRATION_TARGET_Y_PIXELS[2] - screenFirstPointY;

    calculatedCalibration.screenXFromRawX =
        (screenFirstToSecondDeltaX * rawFirstToThirdDeltaY -
         screenFirstToThirdDeltaX * rawFirstToSecondDeltaY) /
        rawCalibrationTriangleAreaTwice;
    calculatedCalibration.screenXFromRawY =
        (rawFirstToSecondDeltaX * screenFirstToThirdDeltaX -
         rawFirstToThirdDeltaX * screenFirstToSecondDeltaX) /
        rawCalibrationTriangleAreaTwice;
    calculatedCalibration.screenXOffset =
        screenFirstPointX -
        calculatedCalibration.screenXFromRawX * rawFirstPointX -
        calculatedCalibration.screenXFromRawY * rawFirstPointY;

    calculatedCalibration.screenYFromRawX =
        (screenFirstToSecondDeltaY * rawFirstToThirdDeltaY -
         screenFirstToThirdDeltaY * rawFirstToSecondDeltaY) /
        rawCalibrationTriangleAreaTwice;
    calculatedCalibration.screenYFromRawY =
        (rawFirstToSecondDeltaX * screenFirstToThirdDeltaY -
         rawFirstToThirdDeltaX * screenFirstToSecondDeltaY) /
        rawCalibrationTriangleAreaTwice;
    calculatedCalibration.screenYOffset =
        screenFirstPointY -
        calculatedCalibration.screenYFromRawX * rawFirstPointX -
        calculatedCalibration.screenYFromRawY * rawFirstPointY;

    return isfinite(calculatedCalibration.screenXFromRawX) &&
           isfinite(calculatedCalibration.screenXFromRawY) &&
           isfinite(calculatedCalibration.screenXOffset) &&
           isfinite(calculatedCalibration.screenYFromRawX) &&
           isfinite(calculatedCalibration.screenYFromRawY) &&
           isfinite(calculatedCalibration.screenYOffset);
}

void continueWithoutTouchCalibration() {
    resistiveTouchCalibrationIsValid = false;
    resistiveTouchPressHasBeenHandled = false;
    Serial.println("AVVISO: touch non verificato; resta disponibile il pulsante BOOT.");
    drawTouchCalibrationBypassed();
    waitWhileServicingMeasurements(STARTUP_TOUCH_BYPASS_MESSAGE_DURATION_MILLISECONDS);
}

} // namespace

void initializeTouchscreen() {
    // GPIO36 e solo ingresso: si usa INPUT senza attivare una pull-up interna non disponibile.
    pinMode(XPT2046_TOUCH_INTERRUPT_PIN, INPUT);
    resistiveTouchController.setCal(
        XPT2046_RAW_COORDINATE_MINIMUM,
        XPT2046_RAW_COORDINATE_MAXIMUM,
        XPT2046_RAW_COORDINATE_MINIMUM,
        XPT2046_RAW_COORDINATE_MAXIMUM,
        static_cast<uint16_t>(temperatureDisplay.width()),
        static_cast<uint16_t>(temperatureDisplay.height()),
        false); // La calibrazione iniziale e neutra; i tre tocchi calcolano la trasformazione effettiva.
    resistiveTouchController.setRotation(XPT2046_TOUCH_DISPLAY_ROTATION);
    resistiveTouchCalibrationIsValid = false; // Ogni avvio richiede nuovi tre tocchi guidati.
    resistiveTouchPressHasBeenHandled = false;
}

void runStartupTouchExperience() {
    while (true) {
        RawTouchCalibrationPoint capturedCalibrationPoints[STARTUP_TOUCH_CALIBRATION_TARGET_COUNT]; // I tre tocchi grezzi da associare ai bersagli.

        for (uint8_t calibrationTargetIndex = 0;
             calibrationTargetIndex < STARTUP_TOUCH_CALIBRATION_TARGET_COUNT;
             ++calibrationTargetIndex) {
            bool calibrationTargetWasCaptured = false; // Consente di riprovare il punto corrente senza perdere quelli gia validi.

            while (!calibrationTargetWasCaptured) {
                const TouchTargetCaptureResult captureResult =
                    captureTouchCalibrationTarget(
                        calibrationTargetIndex,
                        capturedCalibrationPoints[calibrationTargetIndex]);

                if (captureResult == TouchTargetCaptureResult::Captured) {
                    calibrationTargetWasCaptured = true;
                    continue; // Il punto e acquisito; si passa al bersaglio successivo.
                }

                if (captureResult == TouchTargetCaptureResult::BootButtonPressed) {
                    continueWithoutTouchCalibration();
                    return;
                }

                const bool touchWasDetected =
                    captureResult == TouchTargetCaptureResult::NoUsableSamples;
                if (captureResult == TouchTargetCaptureResult::TimedOut) {
                    Serial.printf(
                        "Touch: timeout sul bersaglio %u/%u.\n",
                        static_cast<unsigned int>(calibrationTargetIndex + 1),
                        static_cast<unsigned int>(STARTUP_TOUCH_CALIBRATION_TARGET_COUNT));
                }

                drawTouchCalibrationTargetRetryPrompt(
                    calibrationTargetIndex,
                    touchWasDetected);
                const TouchRecoveryChoice recoveryChoice =
                    waitForTouchRetryOrBootButton(TouchRecoveryChoice::RetryCurrentTarget);
                if (recoveryChoice == TouchRecoveryChoice::ContinueWithBootButton) {
                    continueWithoutTouchCalibration();
                    return;
                }
            }
        }

        TouchScreenCalibrationTransform newTouchCalibration; // Risultato temporaneo, pubblicato solo se tutti i controlli passano.
        if (calculateTouchScreenCalibration(capturedCalibrationPoints, newTouchCalibration)) {
            resistiveTouchCalibration = newTouchCalibration;
            resistiveTouchCalibrationIsValid = true;
            resistiveTouchPressHasBeenHandled = false;
            Serial.println("Touch verificato e calibrato per questa accensione.");
            drawTouchCalibrationSuccess();
            waitWhileServicingMeasurements(STARTUP_TOUCH_SUCCESS_MESSAGE_DURATION_MILLISECONDS);
            return;
        }

        // Tre coordinate quasi allineate non permettono di correggere entrambe le direzioni dello schermo.
        Serial.println("Touch: i tre punti non definiscono una calibrazione stabile.");
        drawTouchCalibrationRecoveryPrompt();
        const TouchRecoveryChoice recoveryChoice =
            waitForTouchRetryOrBootButton(TouchRecoveryChoice::RetryAllTargets);
        if (recoveryChoice == TouchRecoveryChoice::ContinueWithBootButton) {
            continueWithoutTouchCalibration();
            return;
        }
    }
}

static bool readCalibratedTouchPosition(
    int16_t& touchScreenX,
    int16_t& touchScreenY) {
    if (!resistiveTouchCalibrationIsValid ||
        digitalRead(XPT2046_TOUCH_INTERRUPT_PIN) != LOW ||
        !resistiveTouchController.Pressed()) {
        return false;
    }

    const int16_t rawTouchX =
        static_cast<int16_t>(resistiveTouchController.RawX());
    const int16_t rawTouchY =
        static_cast<int16_t>(resistiveTouchController.RawY());
    const float calibratedScreenX =
        resistiveTouchCalibration.screenXFromRawX * rawTouchX +
        resistiveTouchCalibration.screenXFromRawY * rawTouchY +
        resistiveTouchCalibration.screenXOffset;
    const float calibratedScreenY =
        resistiveTouchCalibration.screenYFromRawX * rawTouchX +
        resistiveTouchCalibration.screenYFromRawY * rawTouchY +
        resistiveTouchCalibration.screenYOffset;

    if (!isfinite(calibratedScreenX) || !isfinite(calibratedScreenY) ||
        calibratedScreenX < 0 ||
        calibratedScreenX >= temperatureDisplay.width() ||
        calibratedScreenY < 0 ||
        calibratedScreenY >= temperatureDisplay.height()) {
        return false;
    }

    touchScreenX = static_cast<int16_t>(lroundf(calibratedScreenX));
    touchScreenY = static_cast<int16_t>(lroundf(calibratedScreenY));
    return true;
}

void runStartupDhtSensorModelSelection() {
    if (startupExternalSensorBusCheckResult.mode != ExternalSensorBusMode::Dht) {
        return;
    }

    if (!resistiveTouchCalibrationIsValid) {
        Serial.println(
            "DHT: modello non scelto perche il touch non e calibrato; misure disattivate.");
        drawDhtSensorModelSelectionResult(
            DHT_MODEL_SELECTION_NO_TOUCH_MESSAGE);
        waitWhileServicingMeasurements(DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS);
        return;
    }

    drawDhtSensorModelSelectionScreen();
    const uint32_t selectionWaitStartMilliseconds = millis();
    while (static_cast<uint32_t>(
               millis() - selectionWaitStartMilliseconds) <
           DHT_MODEL_SELECTION_TIMEOUT_MILLISECONDS) {
        if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            waitForBootButtonRelease();
            Serial.println(
                "DHT: scelta modello annullata con BOOT; misure disattivate.");
            drawDhtSensorModelSelectionResult(
                DHT_MODEL_SELECTION_BOOT_MESSAGE);
            waitWhileServicingMeasurements(DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS);
            return;
        }

        int16_t touchScreenX = 0;
        int16_t touchScreenY = 0;
        if (readCalibratedTouchPosition(touchScreenX, touchScreenY)) {
            DhtSensorModel selectedDhtModel = DhtSensorModel::NotSelected;
            if (touchScreenY >= DHT_MODEL_SELECTION_BUTTON_Y_PIXELS &&
                touchScreenY <
                    DHT_MODEL_SELECTION_BUTTON_Y_PIXELS +
                        DHT_MODEL_SELECTION_BUTTON_HEIGHT_PIXELS) {
                if (touchScreenX >= DHT_MODEL_SELECTION_DHT11_BUTTON_X_PIXELS &&
                    touchScreenX <
                        DHT_MODEL_SELECTION_DHT11_BUTTON_X_PIXELS +
                            DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS) {
                    selectedDhtModel = DhtSensorModel::Dht11;
                } else if (
                    touchScreenX >= DHT_MODEL_SELECTION_DHT22_BUTTON_X_PIXELS &&
                    touchScreenX <
                        DHT_MODEL_SELECTION_DHT22_BUTTON_X_PIXELS +
                            DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS) {
                    selectedDhtModel = DhtSensorModel::Dht22;
                }
            }

            if (!waitForTouchReleaseOrBootButton()) {
                Serial.println(
                    "DHT: scelta modello annullata con BOOT; misure disattivate.");
                drawDhtSensorModelSelectionResult(
                    DHT_MODEL_SELECTION_BOOT_MESSAGE);
                waitWhileServicingMeasurements(DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS);
                return;
            }

            if (selectedDhtModel != DhtSensorModel::NotSelected) {
                if (initializeSelectedDhtSensor(
                        selectedDhtModel,
                        getExternalSensorSingleWireDataPin(
                            startupExternalSensorBusCheckResult.pinMapping))) {
                    drawDhtSensorModelSelectionResult(
                        DHT_MODEL_SELECTION_COMPLETE_MESSAGE);
                    waitWhileServicingMeasurements(DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS);
                    return;
                }

                Serial.println(
                    "DHT: impossibile configurare il modello scelto; misure disattivate.");
                drawDhtSensorModelSelectionResult(
                    DHT_MODEL_SELECTION_ERROR_MESSAGE);
                waitWhileServicingMeasurements(DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS);
                return;
            }
        }

        waitWhileServicingMeasurements(STARTUP_TOUCH_WAIT_POLL_INTERVAL_MILLISECONDS);
    }

    Serial.println(
        "DHT: timeout scelta modello; misure disattivate fino al prossimo avvio.");
    drawDhtSensorModelSelectionResult(DHT_MODEL_SELECTION_TIMEOUT_MESSAGE);
    waitWhileServicingMeasurements(DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS);
}

void runStartupNoSensorRecovery() {
    while (startupExternalSensorBusCheckResult.mode ==
           ExternalSensorBusMode::NoSensorDetected) {
        drawStartupNoSensorScreen();
        bool newSearchWasRequested = false;

        while (!newSearchWasRequested) {
            if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
                waitForBootButtonRelease();
                newSearchWasRequested = true;
                Serial.println("CN1: nuova ricerca richiesta con BOOT.");
                break;
            }

            if (resistiveTouchCalibrationIsValid) {
                int16_t touchScreenX = 0;
                int16_t touchScreenY = 0;
                if (readCalibratedTouchPosition(touchScreenX, touchScreenY)) {
                    const bool touchIsInsideStartButton =
                        touchScreenX >= STARTUP_NO_SENSOR_START_BUTTON_X_PIXELS &&
                        touchScreenX <
                            STARTUP_NO_SENSOR_START_BUTTON_X_PIXELS +
                                STARTUP_NO_SENSOR_START_BUTTON_WIDTH_PIXELS &&
                        touchScreenY >= STARTUP_NO_SENSOR_START_BUTTON_Y_PIXELS &&
                        touchScreenY <
                            STARTUP_NO_SENSOR_START_BUTTON_Y_PIXELS +
                                STARTUP_NO_SENSOR_START_BUTTON_HEIGHT_PIXELS;
                    const bool touchWasReleasedNormally =
                        waitForTouchReleaseOrBootButton();
                    if (touchIsInsideStartButton || !touchWasReleasedNormally) {
                        newSearchWasRequested = true;
                        Serial.println("CN1: nuova ricerca richiesta dal touch.");
                    }
                }
            }

            waitWhileServicingMeasurements(STARTUP_NO_SENSOR_POLL_INTERVAL_MILLISECONDS);
        }

        drawStartupSensorSearchProgressScreen();
        initializeTemperatureSensorBus();
        if (startupExternalSensorBusCheckResult.mode ==
            ExternalSensorBusMode::NoSensorDetected) {
            continue;
        }

        drawStartupSummaryScreen(
            startupMicroSdCardCheckResult,
            startupExternalSensorBusCheckResult);
        waitWhileServicingMeasurements(STARTUP_SENSOR_RETRY_SUMMARY_DURATION_MILLISECONDS);
        runStartupDhtSensorModelSelection();
    }
}

void updateTouchscreen() {
    if (!resistiveTouchCalibrationIsValid) {
        return; // Coordinate non calibrate non devono attivare comandi dell'interfaccia.
    }

    if (digitalRead(XPT2046_TOUCH_INTERRUPT_PIN) != LOW) {
        resistiveTouchPressHasBeenHandled = false; // Un nuovo tocco e ammesso solo dopo il rilascio del precedente.
        return;
    }

    if (resistiveTouchPressHasBeenHandled) {
        return; // Un dito mantenuto sullo schermo genera una sola attivazione.
    }

    if (!resistiveTouchController.Pressed()) {
        return; // La libreria attende una lettura stabile prima di autorizzare il comando.
    }
    resistiveTouchPressHasBeenHandled = true;

    const int16_t rawTouchX =
        static_cast<int16_t>(resistiveTouchController.RawX()); // Coordinata grezza X prima della trasformazione di calibrazione.
    const int16_t rawTouchY =
        static_cast<int16_t>(resistiveTouchController.RawY()); // Coordinata grezza Y prima della trasformazione di calibrazione.
    const float calibratedScreenX =
        resistiveTouchCalibration.screenXFromRawX * rawTouchX +
        resistiveTouchCalibration.screenXFromRawY * rawTouchY +
        resistiveTouchCalibration.screenXOffset; // Applica la calibrazione X ricavata dai tre bersagli.
    const float calibratedScreenY =
        resistiveTouchCalibration.screenYFromRawX * rawTouchX +
        resistiveTouchCalibration.screenYFromRawY * rawTouchY +
        resistiveTouchCalibration.screenYOffset; // Applica la calibrazione Y ricavata dai tre bersagli.

    if (!isfinite(calibratedScreenX) || !isfinite(calibratedScreenY) ||
        calibratedScreenX < 0 ||
        calibratedScreenX >= temperatureDisplay.width() ||
        calibratedScreenY < 0 ||
        calibratedScreenY >= temperatureDisplay.height()) {
        return; // Un punto fuori dallo schermo non corrisponde a un comando valido.
    }

    const int16_t touchScreenX = static_cast<int16_t>(lroundf(calibratedScreenX)); // Posizione X arrotondata ai pixel interi.
    const int16_t touchScreenY = static_cast<int16_t>(lroundf(calibratedScreenY)); // Posizione Y arrotondata ai pixel interi.
    const bool controlBarViewButtonWasPressed =
        touchScreenX >= CONTROL_BAR_VIEW_BUTTON_X_PIXELS &&
        touchScreenX < CONTROL_BAR_VIEW_BUTTON_X_PIXELS +
                           CONTROL_BAR_VIEW_BUTTON_WIDTH_PIXELS &&
        touchScreenY >= CONTROL_BAR_BUTTON_Y_PIXELS &&
        touchScreenY < CONTROL_BAR_BUTTON_Y_PIXELS + CONTROL_BAR_BUTTON_HEIGHT_PIXELS;
    const bool controlBarSettingsButtonWasPressed =
        touchScreenX >= CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS &&
        touchScreenX < CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS +
                           CONTROL_BAR_SETTINGS_BUTTON_WIDTH_PIXELS &&
        touchScreenY >= CONTROL_BAR_BUTTON_Y_PIXELS &&
        touchScreenY < CONTROL_BAR_BUTTON_Y_PIXELS + CONTROL_BAR_BUTTON_HEIGHT_PIXELS;

    if (controlBarSettingsButtonWasPressed) {
        technicalMenuIsActive = !technicalMenuIsActive; // L'ingranaggio apre o chiude il menu tecnico.
        temperatureDisplayNeedsRedraw = true;
    } else if (controlBarViewButtonWasPressed) {
        graphViewIsActive = !graphViewIsActive; // Il pulsante sinistro seleziona l'altra vista.
        technicalMenuIsActive = false; // Se il menu e aperto, la selezione porta direttamente alla vista scelta.
        temperatureDisplayNeedsRedraw = true;
    }
}
