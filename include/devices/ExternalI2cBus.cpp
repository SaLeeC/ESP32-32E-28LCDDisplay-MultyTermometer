/*
 * Modulo di scansione I2C alternativo ai bus OneWire e DHT sul connettore CN1.
 *
 * La scansione usa la mappatura selezionata dal ciclo corrente e lascia il bus
 * inizializzato soltanto quando il firmware ha trovato un dispositivo I2C.
 */

uint8_t getExternalSensorSingleWireDataPin(ExternalSensorPinMapping pinMapping) {
    return pinMapping == ExternalSensorPinMapping::Standard
               ? EXTERNAL_SENSOR_PRIMARY_GPIO_PIN
               : EXTERNAL_SENSOR_SECONDARY_GPIO_PIN;
}

uint8_t getExternalSensorI2cSdaPin(ExternalSensorPinMapping pinMapping) {
    return pinMapping == ExternalSensorPinMapping::Standard
               ? EXTERNAL_SENSOR_I2C_SDA_PIN
               : EXTERNAL_SENSOR_I2C_SCL_PIN;
}

uint8_t getExternalSensorI2cSclPin(ExternalSensorPinMapping pinMapping) {
    return pinMapping == ExternalSensorPinMapping::Standard
               ? EXTERNAL_SENSOR_I2C_SCL_PIN
               : EXTERNAL_SENSOR_I2C_SDA_PIN;
}

static void clearExternalI2cSensorRegistry() {
    externalI2cSensorRecordCount = 0;
    for (uint8_t sensorRecordIndex = 0;
         sensorRecordIndex < MAXIMUM_EXTERNAL_I2C_SENSOR_COUNT;
         ++sensorRecordIndex) {
        ExternalI2cSensorRecord& currentSensorRecord =
            externalI2cSensorRecords[sensorRecordIndex];
        currentSensorRecord.i2cAddress = 0;
        currentSensorRecord.catalogCandidateCount = 0;
        currentSensorRecord.identifiedCharacteristics = nullptr;
        for (size_t catalogEntryIndex = 0;
             catalogEntryIndex < EXTERNAL_I2C_SENSOR_CATALOG_ENTRY_COUNT;
             ++catalogEntryIndex) {
            currentSensorRecord.catalogCandidateEntries[catalogEntryIndex] = nullptr;
        }
    }
}

static void registerExternalI2cSensorAddress(uint8_t i2cAddress) {
    ExternalI2cSensorRecord& currentSensorRecord =
        externalI2cSensorRecords[externalI2cSensorRecordCount];
    currentSensorRecord.i2cAddress = i2cAddress;
    currentSensorRecord.catalogCandidateCount = 0;
    currentSensorRecord.identifiedCharacteristics = nullptr;

    for (size_t catalogEntryIndex = 0;
         catalogEntryIndex < EXTERNAL_I2C_SENSOR_CATALOG_ENTRY_COUNT;
         ++catalogEntryIndex) {
        const ExternalI2cSensorCatalogEntry& currentCatalogEntry =
            EXTERNAL_I2C_SENSOR_CATALOG[catalogEntryIndex];
        if (i2cAddress < currentCatalogEntry.firstAddress ||
            i2cAddress > currentCatalogEntry.lastAddress) {
            continue;
        }

        currentSensorRecord.catalogCandidateEntries[
            currentSensorRecord.catalogCandidateCount] = &currentCatalogEntry;
        ++currentSensorRecord.catalogCandidateCount;
    }

    ++externalI2cSensorRecordCount;
    Serial.printf(
        "Sensore I2C #%u registrato a 0x%02X con %u candidato/i catalogo.\n",
        static_cast<unsigned int>(externalI2cSensorRecordCount),
        static_cast<unsigned int>(i2cAddress),
        static_cast<unsigned int>(currentSensorRecord.catalogCandidateCount));

    for (uint8_t candidateIndex = 0;
         candidateIndex < currentSensorRecord.catalogCandidateCount;
         ++candidateIndex) {
        Serial.printf(
            "  Candidato: %s.\n",
            currentSensorRecord.catalogCandidateEntries[candidateIndex]->familyName);
    }
    if (currentSensorRecord.catalogCandidateCount == 0) {
        Serial.println("  Nessuna famiglia corrispondente nel catalogo.");
    }
}

void initializeExternalI2cBusAndScan(
    ExternalSensorBusCheckResult& externalSensorBusCheckResult) {
    if (externalI2cBusIsInitialized) {
        if (!Wire.end()) {
            externalSensorBusCheckResult.mode =
                ExternalSensorBusMode::I2cInitializationError;
            Serial.println("Errore: impossibile rilasciare il bus I2C prima della riconfigurazione.");
            return;
        }
        externalI2cBusIsInitialized = false;
    }

    clearExternalI2cSensorRegistry();
    externalSensorBusCheckResult.mode = ExternalSensorBusMode::I2c;
    externalSensorBusCheckResult.dhtModel = DhtSensorModel::NotSelected;
    const uint8_t sdaPin =
        getExternalSensorI2cSdaPin(externalSensorBusCheckResult.pinMapping);
    const uint8_t sclPin =
        getExternalSensorI2cSclPin(externalSensorBusCheckResult.pinMapping);
    externalSensorBusCheckResult.i2cDeviceCount = 0;
    for (uint8_t addressIndex = 0;
         addressIndex < MAXIMUM_EXTERNAL_I2C_DEVICE_COUNT;
         ++addressIndex) {
        externalSensorBusCheckResult.i2cDeviceAddresses[addressIndex] = 0;
    }

    // Rilascia i pin dopo il tentativo OneWire; Wire li configura poi come SDA e SCL.
    pinMode(sdaPin, INPUT);
    pinMode(sclPin, INPUT);
    Serial.printf(
        "Nessun dispositivo OneWire o DHT: avvio I2C su SDA GPIO%u e SCL GPIO%u.\n",
        static_cast<unsigned int>(sdaPin),
        static_cast<unsigned int>(sclPin));

    if (!Wire.begin(
            sdaPin,
            sclPin,
            EXTERNAL_SENSOR_I2C_FREQUENCY_HERTZ)) {
        if (!Wire.end()) {
            Serial.println(
                "Errore: impossibile rilasciare Wire dopo il fallimento di begin.");
        }
        externalSensorBusCheckResult.mode =
            ExternalSensorBusMode::I2cInitializationError;
        Serial.println("Errore: inizializzazione del bus I2C non riuscita.");
        return;
    }
    externalI2cBusIsInitialized = true;

    bool sensorRecordLimitWarningWasPrinted = false;
    for (uint16_t addressToCheck = EXTERNAL_SENSOR_I2C_FIRST_VALID_ADDRESS;
         addressToCheck <= EXTERNAL_SENSOR_I2C_LAST_VALID_ADDRESS;
         ++addressToCheck) {
        const uint8_t currentI2cDeviceAddress =
            static_cast<uint8_t>(addressToCheck);
        Wire.beginTransmission(currentI2cDeviceAddress);
        const uint8_t i2cTransmissionStatus = Wire.endTransmission();

        if (i2cTransmissionStatus != 0) {
            continue; // Un indirizzo senza risposta non identifica un dispositivo presente.
        }

        if (externalSensorBusCheckResult.i2cDeviceCount <
            MAXIMUM_EXTERNAL_I2C_DEVICE_COUNT) {
            externalSensorBusCheckResult.i2cDeviceAddresses[
                externalSensorBusCheckResult.i2cDeviceCount] =
                currentI2cDeviceAddress;
            ++externalSensorBusCheckResult.i2cDeviceCount;
        }

        Serial.printf(
            "Dispositivo I2C rilevato all'indirizzo 0x%02X.\n",
            static_cast<unsigned int>(currentI2cDeviceAddress));

        if (externalI2cSensorRecordCount < MAXIMUM_EXTERNAL_I2C_SENSOR_COUNT) {
            registerExternalI2cSensorAddress(currentI2cDeviceAddress);
        } else if (!sensorRecordLimitWarningWasPrinted) {
            Serial.printf(
                "Limite registro sensori I2C raggiunto (%u); gli altri indirizzi restano nel risultato della scansione.\n",
                static_cast<unsigned int>(MAXIMUM_EXTERNAL_I2C_SENSOR_COUNT));
            sensorRecordLimitWarningWasPrinted = true;
        }
    }

    Serial.printf(
        "Scansione I2C completata: %u dispositivo/i rilevato/i.\n",
        static_cast<unsigned int>(externalSensorBusCheckResult.i2cDeviceCount));
}
