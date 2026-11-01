/*
 * Modulo delle sonde DS18B20 sul bus OneWire.
 *
 * La ricerca dei dispositivi avviene una sola volta durante l'avvio. Ogni sonda
 * viene poi letta usando il suo indirizzo ROM, cosi due sonde sullo stesso filo
 * dati restano identificabili. Le conversioni sono non bloccanti: il programma
 * le avvia, continua a gestire display e pulsante e legge il risultato dopo 750 ms.
 */

static void clearOneWireSensorRegistry() {
    oneWireSensorRecordCount = 0;
    for (uint8_t sensorRecordIndex = 0;
         sensorRecordIndex < MAXIMUM_ONEWIRE_SENSOR_COUNT;
         ++sensorRecordIndex) {
        OneWireSensorRecord& currentSensorRecord =
            oneWireSensorRecords[sensorRecordIndex];
        for (uint8_t addressByteIndex = 0;
             addressByteIndex < sizeof(DeviceAddress);
             ++addressByteIndex) {
            currentSensorRecord.romAddress[addressByteIndex] = 0;
        }
        currentSensorRecord.catalogCandidateCount = 0;
        currentSensorRecord.identifiedCharacteristics = nullptr;
        for (size_t catalogEntryIndex = 0;
             catalogEntryIndex < ONE_WIRE_SENSOR_CATALOG_ENTRY_COUNT;
             ++catalogEntryIndex) {
            currentSensorRecord.catalogCandidateEntries[catalogEntryIndex] = nullptr;
        }
    }
}

static void registerOneWireSensorAddress(
    const DeviceAddress& discoveredRomAddress) {
    OneWireSensorRecord& currentSensorRecord =
        oneWireSensorRecords[oneWireSensorRecordCount];
    memcpy(
        currentSensorRecord.romAddress,
        discoveredRomAddress,
        sizeof(DeviceAddress));
    currentSensorRecord.catalogCandidateCount = 0;
    currentSensorRecord.identifiedCharacteristics = nullptr;

    for (size_t catalogEntryIndex = 0;
         catalogEntryIndex < ONE_WIRE_SENSOR_CATALOG_ENTRY_COUNT;
         ++catalogEntryIndex) {
        const OneWireSensorCatalogEntry& currentCatalogEntry =
            ONE_WIRE_SENSOR_CATALOG[catalogEntryIndex];
        if (discoveredRomAddress[0] != currentCatalogEntry.familyCode) {
            continue;
        }

        currentSensorRecord.catalogCandidateEntries[
            currentSensorRecord.catalogCandidateCount] = &currentCatalogEntry;
        ++currentSensorRecord.catalogCandidateCount;
    }

    ++oneWireSensorRecordCount;
    Serial.printf(
        "Registro OneWire #%u ROM:",
        static_cast<unsigned int>(oneWireSensorRecordCount));
    for (uint8_t addressByteIndex = 0;
         addressByteIndex < sizeof(DeviceAddress);
         ++addressByteIndex) {
        Serial.printf(
            " %02X",
            static_cast<unsigned int>(
                currentSensorRecord.romAddress[addressByteIndex]));
    }
    Serial.printf(
        " - famiglia 0x%02X, %u candidato/i catalogo.\n",
        static_cast<unsigned int>(currentSensorRecord.romAddress[0]),
        static_cast<unsigned int>(currentSensorRecord.catalogCandidateCount));

    for (uint8_t candidateIndex = 0;
         candidateIndex < currentSensorRecord.catalogCandidateCount;
         ++candidateIndex) {
        Serial.printf(
            "  Candidato: %s.\n",
            currentSensorRecord.catalogCandidateEntries[candidateIndex]->familyName);
    }
    if (currentSensorRecord.catalogCandidateCount == 0) {
        Serial.println("  Codice famiglia non presente nel catalogo.");
    }
}

/*
 * Rilascia Wire prima di riconfigurare CN1 per OneWire o DHT.
 * Parametri: nessuno.
 * Restituisce: true se il bus e gia fermo o e stato rilasciato correttamente.
 * Effetti: aggiorna lo stato condiviso dell'inizializzazione I2C.
 */
static bool endExternalI2cBusIfInitialized() {
    if (!externalI2cBusIsInitialized) {
    return true;
    }

    if (!Wire.end()) {
    Serial.println(
        "Errore: impossibile rilasciare il bus I2C prima di cambiare modalita.");
    return false;
    }

    externalI2cBusIsInitialized = false;
    return true;
}

void initializeTemperatureSensorBus() {
    if (!endExternalI2cBusIfInitialized()) {
    startupExternalSensorBusCheckResult.mode =
        ExternalSensorBusMode::NoSensorDetected;
    return;
    }

    clearOneWireSensorRegistry();
    activeTemperatureProbeCount = 0;
    externalI2cSensorRecordCount = 0;
    activeDhtExternalSensor = nullptr;
    latestDhtRelativeHumidityPercent = NAN;
    dhtMeasurementHasBeenAttempted = false;
    temperatureConversionIsPending = false;
    startupExternalSensorBusCheckResult.mode = ExternalSensorBusMode::NotChecked;
    startupExternalSensorBusCheckResult.oneWireDeviceCount = 0;
    startupExternalSensorBusCheckResult.i2cDeviceCount = 0;
    startupExternalSensorBusCheckResult.dhtModel = DhtSensorModel::NotSelected;
    startupExternalSensorBusCheckResult.pinMapping =
    ExternalSensorPinMapping::Standard;
    for (uint8_t addressIndex = 0;
     addressIndex < MAXIMUM_EXTERNAL_I2C_DEVICE_COUNT;
     ++addressIndex) {
    startupExternalSensorBusCheckResult.i2cDeviceAddresses[addressIndex] = 0;
    }

    bool sensorWasDetected = false;
    for (uint8_t pinMappingIndex = 0; pinMappingIndex < 2; ++pinMappingIndex) {
    const ExternalSensorPinMapping currentPinMapping =
        pinMappingIndex == 0
            ? ExternalSensorPinMapping::Standard
            : ExternalSensorPinMapping::Swapped;
    const uint8_t singleWireDataPin =
        getExternalSensorSingleWireDataPin(currentPinMapping);
    const uint8_t i2cSdaPin =
        getExternalSensorI2cSdaPin(currentPinMapping);
    const uint8_t i2cSclPin =
        getExternalSensorI2cSclPin(currentPinMapping);

    if (!endExternalI2cBusIfInitialized()) {
        startupExternalSensorBusCheckResult.mode =
            ExternalSensorBusMode::NoSensorDetected;
        break;
    }

    startupExternalSensorBusCheckResult.pinMapping = currentPinMapping;
    startupExternalSensorBusCheckResult.mode =
        ExternalSensorBusMode::NotChecked;
    startupExternalSensorBusCheckResult.oneWireDeviceCount = 0;
    startupExternalSensorBusCheckResult.i2cDeviceCount = 0;
    activeTemperatureProbeCount = 0;
    clearOneWireSensorRegistry();

    Serial.printf(
        "Ricerca CN1 %u/2: OneWire/DHT GPIO%u, I2C SDA%u/SCL%u.\n",
        static_cast<unsigned int>(pinMappingIndex + 1),
        static_cast<unsigned int>(singleWireDataPin),
        static_cast<unsigned int>(i2cSdaPin),
        static_cast<unsigned int>(i2cSclPin));

    ds18b20OneWireBus.begin(singleWireDataPin);
    startupExternalSensorBusCheckResult.dhtModel =
        DhtSensorModel::NotSelected;
    ds18b20TemperatureSensors.begin();
    const uint8_t oneWireDeviceCount = discoverTemperatureProbes();
    startupExternalSensorBusCheckResult.oneWireDeviceCount =
        oneWireDeviceCount;

    if (oneWireDeviceCount > 0) {
        startupExternalSensorBusCheckResult.mode =
            ExternalSensorBusMode::OneWire;
        Serial.printf(
            "OneWire: %u dispositivo/i rilevato/i su GPIO%u; gli altri protocolli non vengono avviati.\n",
            static_cast<unsigned int>(oneWireDeviceCount),
            static_cast<unsigned int>(singleWireDataPin));
        sensorWasDetected = true;
    } else if (detectDhtSensorOnExternalBus(singleWireDataPin)) {
        startupExternalSensorBusCheckResult.mode =
            ExternalSensorBusMode::Dht;
        Serial.println(
            "DHT: frame e checksum validi; scegliere DHT11 o DHT22 prima delle misure.");
        sensorWasDetected = true;
    } else {
        initializeExternalI2cBusAndScan(
            startupExternalSensorBusCheckResult);
        sensorWasDetected =
            startupExternalSensorBusCheckResult.mode ==
                ExternalSensorBusMode::I2c &&
            startupExternalSensorBusCheckResult.i2cDeviceCount > 0;
    }

    if (sensorWasDetected) {
        break;
    }

    if (pinMappingIndex == 0) {
        Serial.println(
            "Nessun sensore nella configurazione standard; nuovo tentativo con GPIO27 e GPIO22 scambiati.");
    }
    }

    if (!sensorWasDetected) {
    endExternalI2cBusIfInitialized();
    startupExternalSensorBusCheckResult.mode =
        ExternalSensorBusMode::NoSensorDetected;
    startupExternalSensorBusCheckResult.dhtModel =
        DhtSensorModel::NotSelected;
    Serial.println(
        "Nessun sensore rilevato dopo entrambi i tentativi GPIO: OneWire, DHT e I2C non hanno risposto.");
    return;
    }

    if (startupExternalSensorBusCheckResult.mode !=
        ExternalSensorBusMode::OneWire ||
    activeTemperatureProbeCount == 0) {
    return; // DHT e I2C usano driver separati e non devono ricevere comandi OneWire.
    }

    ds18b20TemperatureSensors.setResolution(DS18B20_RESOLUTION_BITS); // Imposta la precisione della misura e il relativo tempo di conversione.
    ds18b20TemperatureSensors.setWaitForConversion(false); // Permette al programma di continuare mentre le sonde stanno misurando.
    ds18b20TemperatureSensors.requestTemperatures(); // Avvia la prima misura per tutte le sonde, senza attenderne qui il completamento.
    lastTemperatureConversionStartMilliseconds = millis(); // Da questo istante decorre l'attesa della conversione.
    temperatureConversionIsPending = true; // Il ciclo principale leggera i risultati quando saranno pronti.
}

uint8_t discoverTemperatureProbes() {
    DeviceAddress discoveredRomAddress; // Buffer temporaneo in cui la libreria copia l'indirizzo ROM trovato.
    uint8_t recognizedDs18b20ProbeCount = 0; // Conta le DS18B20 trovate, incluse quelle oltre il limite supportato.
    const uint8_t oneWireDeviceCount = ds18b20TemperatureSensors.getDeviceCount(); // Numero di dispositivi rilevati sul bus OneWire.
    bool oneWireRecordLimitWarningWasPrinted = false; // Evita messaggi ripetuti se il bus supera la capienza del registro.

    // La ricerca avviene solo all'avvio: dopo aver aggiunto o rimosso sonde occorre riavviare.
    for (uint8_t deviceIndex = 0; deviceIndex < oneWireDeviceCount; ++deviceIndex) { // Indice assegnato dalla libreria al dispositivo.
        if (!ds18b20TemperatureSensors.getAddress(discoveredRomAddress, deviceIndex)) {
            continue; // Salta ROM non leggibili o non validi.
        }

        if (oneWireSensorRecordCount < MAXIMUM_ONEWIRE_SENSOR_COUNT) {
            registerOneWireSensorAddress(discoveredRomAddress);
        } else if (!oneWireRecordLimitWarningWasPrinted) {
            Serial.printf(
                "Limite registro OneWire raggiunto (%u); gli altri dispositivi non vengono registrati.\n",
                static_cast<unsigned int>(MAXIMUM_ONEWIRE_SENSOR_COUNT));
            oneWireRecordLimitWarningWasPrinted = true;
        }

        if (discoveredRomAddress[0] != DS18B20_FAMILY_CODE) {
            continue; // I dispositivi non DS18B20 restano nel registro ma non vengono gestiti come sonde termiche.
        }

        ++recognizedDs18b20ProbeCount;
        if (activeTemperatureProbeCount >= MAXIMUM_PROBE_COUNT) {
            continue; // Conta la sonda, ma non la memorizza se il firmware ha gia raggiunto il suo limite.
        }

        TemperatureProbe& currentProbe = temperatureProbes[activeTemperatureProbeCount]; // Spazio che diventera la prossima sonda S1, S2 e cosi via.
        memcpy(currentProbe.romAddress, discoveredRomAddress, sizeof(DeviceAddress)); // Conserva l'indirizzo necessario a interrogare la sonda corretta.
        currentProbe.latestTemperatureCelsius = NAN; // Segnala che non e ancora stata acquisita una temperatura.

        for (float& historicalTemperatureCelsius : currentProbe.temperatureHistoryCelsius) { // Prepara tutte le posizioni dello storico.
            historicalTemperatureCelsius = NAN; // Evita di mostrare valori casuali presenti in RAM prima delle prime misure.
        }

        // Stampa l'indirizzo ROM per poter associare in modo verificabile S1, S2 e cosi via alle sonde fisiche.
        Serial.printf("S%u ROM:", static_cast<unsigned int>(activeTemperatureProbeCount + 1));
        for (uint8_t addressByteIndex = 0; addressByteIndex < sizeof(DeviceAddress); ++addressByteIndex) { // Mostra tutti i byte dell'indirizzo univoco.
            Serial.printf(" %02X", static_cast<unsigned int>(currentProbe.romAddress[addressByteIndex]));
        }
        Serial.println();
        ++activeTemperatureProbeCount;
    }

    if (recognizedDs18b20ProbeCount > MAXIMUM_PROBE_COUNT) {
        // Le sonde oltre il limite sono ignorate, ma l'utente riceve un avviso sul monitor seriale.
        const unsigned int ignoredProbeCount = recognizedDs18b20ProbeCount - MAXIMUM_PROBE_COUNT; // Numero di sonde trovate ma non gestite.
        Serial.printf("Limite %u sonde: %u sonde ignorate\n",
                      static_cast<unsigned int>(MAXIMUM_PROBE_COUNT),
                      ignoredProbeCount);
    }

    return oneWireDeviceCount; // Include i dispositivi non DS18B20 per impedire il passaggio a un protocollo alternativo.
}

void storeTemperatureHistorySample() {
    for (uint8_t probeIndex = 0; probeIndex < activeTemperatureProbeCount; ++probeIndex) {
        temperatureProbes[probeIndex].temperatureHistoryCelsius[
            nextTemperatureHistoryWriteIndex] =
            temperatureProbes[probeIndex].latestTemperatureCelsius;
    }

    nextTemperatureHistoryWriteIndex =
        (nextTemperatureHistoryWriteIndex + 1) % TEMPERATURE_HISTORY_CAPACITY;
    if (storedTemperatureHistorySampleCount < TEMPERATURE_HISTORY_CAPACITY) {
        ++storedTemperatureHistorySampleCount;
    }

    if (!technicalMenuIsActive) {
        temperatureDisplayNeedsRedraw = true;
    }
}

void updateTemperatureMeasurements(uint32_t currentTimeMilliseconds) {
    if (startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::Dht) {
        updateDhtSensorMeasurements(currentTimeMilliseconds);
        return;
    }

    if (startupExternalSensorBusCheckResult.mode != ExternalSensorBusMode::OneWire ||
        activeTemperatureProbeCount == 0) {
        return; // Senza sonde non ci sono conversioni da avviare o leggere.
    }

    // Se la conversione avviata in precedenza e terminata, legge il risultato di ogni sonda.
    if (temperatureConversionIsPending &&
        static_cast<uint32_t>(currentTimeMilliseconds - lastTemperatureConversionStartMilliseconds) >=
            DS18B20_CONVERSION_DURATION_MILLISECONDS) {
        for (uint8_t probeIndex = 0; probeIndex < activeTemperatureProbeCount; ++probeIndex) { // Legge ogni sonda usando il suo indirizzo ROM.
            float measuredTemperatureCelsius =
                ds18b20TemperatureSensors.getTempC(temperatureProbes[probeIndex].romAddress); // Risultato restituito dalla libreria, in gradi Celsius.

            if (!isfinite(measuredTemperatureCelsius) ||
                measuredTemperatureCelsius == DEVICE_DISCONNECTED_C ||
                measuredTemperatureCelsius < DS18B20_MINIMUM_TEMPERATURE_CELSIUS ||
                measuredTemperatureCelsius > DS18B20_MAXIMUM_TEMPERATURE_CELSIUS) {
                measuredTemperatureCelsius = NAN; // NAN identifica chiaramente un dato che non deve apparire come temperatura valida.
            }

            temperatureProbes[probeIndex].latestTemperatureCelsius = measuredTemperatureCelsius;
        }

        storeTemperatureHistorySample(); // Tutte le sonde condividono la stessa posizione temporale.
        temperatureConversionIsPending = false;
    }

    // Avvia la misura seguente solo se non ce n'e gia una in corso e l'intervallo e trascorso.
    if (!temperatureConversionIsPending &&
        static_cast<uint32_t>(currentTimeMilliseconds - lastTemperatureConversionStartMilliseconds) >=
            TEMPERATURE_SAMPLE_INTERVAL_MILLISECONDS) {
        ds18b20TemperatureSensors.requestTemperatures(); // Le sonde iniziano insieme una nuova conversione.
        lastTemperatureConversionStartMilliseconds = currentTimeMilliseconds; // Memorizza l'istante da cui attendere il risultato.
        temperatureConversionIsPending = true;
    }
}
