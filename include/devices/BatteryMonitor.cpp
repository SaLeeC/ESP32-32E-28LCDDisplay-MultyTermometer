/*
 * Misura la tensione del connettore batteria tramite il partitore resistivo
 * documentato nello schema della scheda. La sola linea BAT_ADC non espone
 * la presenza dei 5 V USB: con una tensione LiPo plausibile non e quindi
 * possibile distinguere batteria sola da USB con batteria collegata.
 */

static uint16_t sampleBatteryAdcVoltageMillivolts(
    const char* measurementPhaseName) {
    uint32_t adcVoltageSumMillivolts = 0; // Somma dei campioni calibrati, in millivolt, usata per calcolare la media.
    uint16_t minimumAdcVoltageMillivolts = UINT16_MAX; // Campione minimo osservato, in millivolt, utile alla prova hardware.
    uint16_t maximumAdcVoltageMillivolts = 0; // Campione massimo osservato, in millivolt, utile alla prova hardware.

    for (uint8_t sampleIndex = 0;
         sampleIndex < BATTERY_VOLTAGE_SAMPLE_COUNT;
         ++sampleIndex) {
        const uint16_t adcVoltageMillivolts = static_cast<uint16_t>(
            analogReadMilliVolts(BATTERY_VOLTAGE_ADC_GPIO_PIN)); // Lettura calibrata del punto centrale del partitore.
        adcVoltageSumMillivolts += adcVoltageMillivolts;
        minimumAdcVoltageMillivolts =
            min(minimumAdcVoltageMillivolts, adcVoltageMillivolts);
        maximumAdcVoltageMillivolts =
            max(maximumAdcVoltageMillivolts, adcVoltageMillivolts);
        delay(BATTERY_VOLTAGE_SAMPLE_INTERVAL_MILLISECONDS);
    }

    const uint16_t averageAdcVoltageMillivolts = static_cast<uint16_t>(
        (adcVoltageSumMillivolts + BATTERY_VOLTAGE_SAMPLE_COUNT / 2U) /
        BATTERY_VOLTAGE_SAMPLE_COUNT); // Media arrotondata della tensione presente su GPIO34.

    Serial.printf(
        "Batteria %s: ADC medio %u mV (min %u, max %u).\n",
        measurementPhaseName,
        static_cast<unsigned int>(averageAdcVoltageMillivolts),
        static_cast<unsigned int>(minimumAdcVoltageMillivolts),
        static_cast<unsigned int>(maximumAdcVoltageMillivolts));

    return averageAdcVoltageMillivolts;
}

StartupBatteryCheckResult measureStartupBatteryVoltage() {
    pinMode(BATTERY_VOLTAGE_ADC_GPIO_PIN, INPUT);
    analogSetPinAttenuation(BATTERY_VOLTAGE_ADC_GPIO_PIN, ADC_11db);
    WiFi.mode(WIFI_OFF);
    digitalWrite(TFT_BL, LOW);
    pinMode(ONBOARD_RGB_LED_RED_GPIO_PIN, INPUT);
    pinMode(ONBOARD_RGB_LED_GREEN_GPIO_PIN, INPUT);
    pinMode(ONBOARD_RGB_LED_BLUE_GPIO_PIN, INPUT);
    delay(BATTERY_MINIMUM_LOAD_STABILIZATION_MILLISECONDS);

    const uint16_t idleAdcVoltageMillivolts =
        sampleBatteryAdcVoltageMillivolts(
            "carico minimo"); // Riferimento con radio e retroilluminazione spente.
    const uint16_t batteryVoltageMillivolts = static_cast<uint16_t>(
        idleAdcVoltageMillivolts *
        BATTERY_VOLTAGE_DIVIDER_MULTIPLIER); // Le due resistenze uguali dimezzano la tensione BAT+.

    digitalWrite(TFT_BL, HIGH);
    pinMode(ONBOARD_RGB_LED_RED_GPIO_PIN, OUTPUT);
    pinMode(ONBOARD_RGB_LED_GREEN_GPIO_PIN, OUTPUT);
    pinMode(ONBOARD_RGB_LED_BLUE_GPIO_PIN, OUTPUT);
    digitalWrite(ONBOARD_RGB_LED_RED_GPIO_PIN, LOW);
    digitalWrite(ONBOARD_RGB_LED_GREEN_GPIO_PIN, LOW);
    digitalWrite(ONBOARD_RGB_LED_BLUE_GPIO_PIN, LOW);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    const int16_t wifiScanStartResult = WiFi.scanNetworks(
        true,
        true); // La scansione asincrona mantiene la radio attiva mentre l'ADC acquisisce i campioni.
    const bool maximumWifiTransmitPowerWasSet =
        wifiScanStartResult == WIFI_SCAN_RUNNING &&
        WiFi.setTxPower(WIFI_POWER_19_5dBm); // La scansione avviata garantisce che la stazione sia pronta ad accettare la potenza massima.
    const bool loadTestWasCompleted =
        maximumWifiTransmitPowerWasSet &&
        wifiScanStartResult == WIFI_SCAN_RUNNING;
    uint16_t maximumLoadAdcVoltageMillivolts = idleAdcVoltageMillivolts; // Se il test non parte conserva il riferimento senza inventare una seconda misura.

    if (loadTestWasCompleted) {
        delay(BATTERY_MAXIMUM_LOAD_STABILIZATION_MILLISECONDS);
        maximumLoadAdcVoltageMillivolts =
            sampleBatteryAdcVoltageMillivolts(
                "carico massimo");
    } else {
        Serial.printf(
            "Batteria: carico massimo incompleto; potenza WiFi %s, scansione codice %d.\n",
            maximumWifiTransmitPowerWasSet ? "OK" : "ERRORE",
            static_cast<int>(wifiScanStartResult));
    }

    WiFi.scanDelete();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    pinMode(ONBOARD_RGB_LED_RED_GPIO_PIN, INPUT);
    pinMode(ONBOARD_RGB_LED_GREEN_GPIO_PIN, INPUT);
    pinMode(ONBOARD_RGB_LED_BLUE_GPIO_PIN, INPUT);

    const uint16_t maximumLoadBatteryVoltageMillivolts = static_cast<uint16_t>(
        maximumLoadAdcVoltageMillivolts *
        BATTERY_VOLTAGE_DIVIDER_MULTIPLIER); // Tensione BAT+ ricostruita con display e radio al massimo.
    const int16_t loadVoltageChangeMillivolts = static_cast<int16_t>(
        static_cast<int32_t>(maximumLoadBatteryVoltageMillivolts) -
        static_cast<int32_t>(batteryVoltageMillivolts)); // Differenza con segno: negativa quando BAT+ cala.

    StartupPowerAssessment powerAssessment =
        StartupPowerAssessment::BatteryNodeVoltageDetected; // Caso normale sul nodo condiviso tra caricatore e batteria.
    if (batteryVoltageMillivolts < BATTERY_NOT_DETECTED_MAXIMUM_MILLIVOLTS) {
        powerAssessment =
            StartupPowerAssessment::UsbWithoutDetectedBattery;
    } else if (batteryVoltageMillivolts > BATTERY_PLAUSIBLE_MAXIMUM_MILLIVOLTS) {
        powerAssessment =
            StartupPowerAssessment::BatteryVoltageOutOfRange;
    }

    Serial.printf(
        "Batteria GPIO%u: BAT+ carico minimo %u mV, carico massimo %u mV, variazione %+d mV.\n",
        static_cast<unsigned int>(BATTERY_VOLTAGE_ADC_GPIO_PIN),
        static_cast<unsigned int>(batteryVoltageMillivolts),
        static_cast<unsigned int>(maximumLoadBatteryVoltageMillivolts),
        static_cast<int>(loadVoltageChangeMillivolts));

    if (powerAssessment ==
        StartupPowerAssessment::BatteryNodeVoltageDetected) {
        Serial.println(
            "Alimentazione: BAT+ plausibile; non dimostra la presenza della batteria perche il caricatore USB a vuoto raggiunge circa 4,19 V.");
    } else if (powerAssessment ==
               StartupPowerAssessment::UsbWithoutDetectedBattery) {
        Serial.println(
            "Alimentazione: batteria non rilevata; con la scheda accesa si deduce alimentazione USB.");
    } else {
        Serial.println(
            "Alimentazione: lettura batteria fuori intervallo; verificare revisione, partitore e calibrazione.");
    }

    return {
        idleAdcVoltageMillivolts,
        batteryVoltageMillivolts,
        maximumLoadBatteryVoltageMillivolts,
        loadVoltageChangeMillivolts,
        loadTestWasCompleted,
        powerAssessment
    };
}
