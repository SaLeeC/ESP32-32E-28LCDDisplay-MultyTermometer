/*
 * Modulo di rilevamento e misura dei sensori DHT single-wire su CN1.
 *
 * Il rilevamento legge un frame e ne verifica il checksum senza interpretare
 * temperatura o umidita. Il driver di misura viene inizializzato solo dopo che
 * l'operatore ha scelto esplicitamente DHT11 o DHT22.
 */

static bool dhtPulseDurationIsPlausible(uint32_t pulseDurationMicroseconds) {
    return pulseDurationMicroseconds >= DHT_SENSOR_MINIMUM_VALID_PULSE_MICROSECONDS &&
           pulseDurationMicroseconds <= DHT_SENSOR_MAXIMUM_VALID_PULSE_MICROSECONDS;
}

bool detectDhtSensorOnExternalBus(uint8_t dataPin) {
    uint8_t receivedDhtFrame[5] = {}; // Cinque byte sono il formato comune usato dai frame DHT supportati.

    pinMode(dataPin, INPUT_PULLUP);
    delay(2); // Lascia stabilizzare la linea prima di iniziare il protocollo.
    digitalWrite(dataPin, LOW);
    pinMode(dataPin, OUTPUT);
    delay(DHT_SENSOR_START_SIGNAL_DURATION_MILLISECONDS);
    pinMode(dataPin, INPUT_PULLUP); // Rilascia la linea per ricevere la risposta del sensore.

    const uint32_t responseLowPulseDurationMicroseconds =
        pulseIn(
            dataPin,
            LOW,
            DHT_SENSOR_PULSE_TIMEOUT_MICROSECONDS);
    const uint32_t responseHighPulseDurationMicroseconds =
        pulseIn(
            dataPin,
            HIGH,
            DHT_SENSOR_PULSE_TIMEOUT_MICROSECONDS);
    bool dhtFrameWasReceived =
        responseLowPulseDurationMicroseconds >=
            DHT_SENSOR_MINIMUM_RESPONSE_PULSE_MICROSECONDS &&
        responseLowPulseDurationMicroseconds <=
            DHT_SENSOR_MAXIMUM_VALID_PULSE_MICROSECONDS &&
        responseHighPulseDurationMicroseconds >=
            DHT_SENSOR_MINIMUM_RESPONSE_PULSE_MICROSECONDS &&
        responseHighPulseDurationMicroseconds <=
            DHT_SENSOR_MAXIMUM_VALID_PULSE_MICROSECONDS;

    for (uint8_t frameBitIndex = 0;
         frameBitIndex < sizeof(receivedDhtFrame) * 8 && dhtFrameWasReceived;
         ++frameBitIndex) {
        const uint32_t dataLowPulseDurationMicroseconds =
            pulseIn(
                dataPin,
                LOW,
                DHT_SENSOR_PULSE_TIMEOUT_MICROSECONDS);
        const uint32_t dataHighPulseDurationMicroseconds =
            pulseIn(
                dataPin,
                HIGH,
                DHT_SENSOR_PULSE_TIMEOUT_MICROSECONDS);

        if (!dhtPulseDurationIsPlausible(dataLowPulseDurationMicroseconds) ||
            !dhtPulseDurationIsPlausible(dataHighPulseDurationMicroseconds)) {
            dhtFrameWasReceived = false;
            break;
        }

        const uint8_t frameByteIndex = frameBitIndex / 8;
        receivedDhtFrame[frameByteIndex] = static_cast<uint8_t>(
            (receivedDhtFrame[frameByteIndex] << 1) |
            (dataHighPulseDurationMicroseconds >=
             DHT_SENSOR_LOGIC_ONE_THRESHOLD_MICROSECONDS));
    }

    pinMode(dataPin, INPUT); // Il pin resta rilasciato se il controllo prosegue in I2C.
    if (!dhtFrameWasReceived) {
        Serial.printf("DHT: nessuna risposta completa valida su GPIO%u.\n", dataPin);
        return false;
    }

    const uint8_t calculatedDhtChecksum = static_cast<uint8_t>(
        receivedDhtFrame[0] +
        receivedDhtFrame[1] +
        receivedDhtFrame[2] +
        receivedDhtFrame[3]);
    if (calculatedDhtChecksum != receivedDhtFrame[4]) {
        Serial.printf(
            "DHT su GPIO%u: frame ricevuto ma checksum non valido (calcolato 0x%02X, ricevuto 0x%02X).\n",
            dataPin,
            static_cast<unsigned int>(calculatedDhtChecksum),
            static_cast<unsigned int>(receivedDhtFrame[4]));
        return false;
    }

    Serial.printf(
        "DHT: risposta e checksum validi su GPIO%u; modello da confermare dall'operatore.\n",
        dataPin);
    return true;
}

const char* getDhtSensorShortModelName(DhtSensorModel dhtSensorModel) {
    switch (dhtSensorModel) {
        case DhtSensorModel::Dht11:
            return DHT_MODEL_SELECTION_DHT11_LABEL;
        case DhtSensorModel::Dht22:
            return DHT_MODEL_SELECTION_DHT22_LABEL;
        case DhtSensorModel::Dht12:
            return "DHT12";
        case DhtSensorModel::Dht21:
            return "DHT21";
        case DhtSensorModel::NotSelected:
            return "DHT";
    }

    return "DHT";
}

static const DhtSensorCatalogEntry* findDhtSensorCatalogEntry(
    DhtSensorModel dhtSensorModel) {
    for (size_t catalogEntryIndex = 0;
         catalogEntryIndex < DHT_SENSOR_CATALOG_ENTRY_COUNT;
         ++catalogEntryIndex) {
        if (DHT_SENSOR_CATALOG[catalogEntryIndex].model == dhtSensorModel) {
            return &DHT_SENSOR_CATALOG[catalogEntryIndex];
        }
    }

    return nullptr;
}

bool initializeSelectedDhtSensor(DhtSensorModel selectedModel, uint8_t dataPin) {
    DHT* selectedDhtDriver = nullptr;
    if (dataPin == EXTERNAL_SENSOR_PRIMARY_GPIO_PIN) {
        if (selectedModel == DhtSensorModel::Dht11) {
            selectedDhtDriver = &dht11ExternalSensorOnPrimaryPin;
        } else if (selectedModel == DhtSensorModel::Dht22) {
            selectedDhtDriver = &dht22ExternalSensorOnPrimaryPin;
        }
    } else if (dataPin == EXTERNAL_SENSOR_SECONDARY_GPIO_PIN) {
        if (selectedModel == DhtSensorModel::Dht11) {
            selectedDhtDriver = &dht11ExternalSensorOnSecondaryPin;
        } else if (selectedModel == DhtSensorModel::Dht22) {
            selectedDhtDriver = &dht22ExternalSensorOnSecondaryPin;
        }
    }

    if (selectedDhtDriver == nullptr) {
        Serial.println("DHT: modello non supportato; misure non avviate.");
        return false;
    }

    selectedDhtDriver->begin();
    activeDhtExternalSensor = selectedDhtDriver;
    startupExternalSensorBusCheckResult.dhtModel = selectedModel;
    activeTemperatureProbeCount = 1;
    TemperatureProbe& dhtTemperatureProbe = temperatureProbes[0];
    for (uint8_t addressByteIndex = 0;
         addressByteIndex < sizeof(DeviceAddress);
         ++addressByteIndex) {
        dhtTemperatureProbe.romAddress[addressByteIndex] = 0;
    }
    dhtTemperatureProbe.latestTemperatureCelsius = NAN;
    for (float& historicalTemperatureCelsius :
         dhtTemperatureProbe.temperatureHistoryCelsius) {
        historicalTemperatureCelsius = NAN;
    }

    latestDhtRelativeHumidityPercent = NAN;
    lastDhtMeasurementMilliseconds = millis();
    dhtMeasurementHasBeenAttempted = false;
    updateMeasurementProcessState();
    temperatureDisplayNeedsRedraw = true;
    Serial.printf(
        "DHT: modello %s selezionato dall'operatore su GPIO%u; misure abilitate.\n",
        getDhtSensorShortModelName(selectedModel),
        dataPin);
    return true;
}

void updateDhtSensorMeasurements(uint32_t currentTimeMilliseconds) {
    const DhtSensorModel selectedDhtModel =
        startupExternalSensorBusCheckResult.dhtModel;
    if (selectedDhtModel == DhtSensorModel::NotSelected ||
        activeDhtExternalSensor == nullptr ||
        activeTemperatureProbeCount == 0 ||
        (dhtMeasurementHasBeenAttempted &&
         static_cast<uint32_t>(
             currentTimeMilliseconds - lastDhtMeasurementMilliseconds) <
             DHT_SENSOR_MINIMUM_READING_INTERVAL_MILLISECONDS)) {
        return;
    }

    float measuredTemperatureCelsius = NAN;
    float measuredRelativeHumidityPercent = NAN;
    measuredRelativeHumidityPercent = activeDhtExternalSensor->readHumidity();
    measuredTemperatureCelsius = activeDhtExternalSensor->readTemperature();

    const DhtSensorCatalogEntry* selectedModelCharacteristics =
        findDhtSensorCatalogEntry(selectedDhtModel);
    if (selectedModelCharacteristics == nullptr) {
        Serial.println("DHT: caratteristiche del modello selezionato non disponibili.");
        return;
    }

    if (!isfinite(measuredTemperatureCelsius) ||
        measuredTemperatureCelsius <
            selectedModelCharacteristics->minimumTemperatureCelsius ||
        measuredTemperatureCelsius >
            selectedModelCharacteristics->maximumTemperatureCelsius) {
        measuredTemperatureCelsius = NAN;
    }
    if (!isfinite(measuredRelativeHumidityPercent) ||
        measuredRelativeHumidityPercent <
            selectedModelCharacteristics->minimumRelativeHumidityPercent ||
        measuredRelativeHumidityPercent >
            selectedModelCharacteristics->maximumRelativeHumidityPercent) {
        measuredRelativeHumidityPercent = NAN;
    }

    temperatureProbes[0].latestTemperatureCelsius = measuredTemperatureCelsius;
    latestDhtRelativeHumidityPercent = measuredRelativeHumidityPercent;
    storeTemperatureHistorySample();
    lastDhtMeasurementMilliseconds = currentTimeMilliseconds;
    dhtMeasurementHasBeenAttempted = true;

    if (isfinite(measuredTemperatureCelsius) &&
        isfinite(measuredRelativeHumidityPercent)) {
        Serial.printf(
            "%s: %.2f C, umidita %.1f %%.\n",
            getDhtSensorShortModelName(selectedDhtModel),
            measuredTemperatureCelsius,
            measuredRelativeHumidityPercent);
    } else {
        Serial.printf(
            "%s: lettura non valida (temperatura %.2f C, umidita %.1f %%).\n",
            getDhtSensorShortModelName(selectedDhtModel),
            measuredTemperatureCelsius,
            measuredRelativeHumidityPercent);
    }
}
