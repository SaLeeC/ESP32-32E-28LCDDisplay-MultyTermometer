/*
 * Modulo delle sonde DS18B20 sul bus OneWire.
 *
 * La ricerca dei dispositivi avviene una sola volta durante l'avvio. Ogni sonda
 * viene poi letta usando il suo indirizzo ROM, cosi due sonde sullo stesso filo
 * dati restano identificabili. Le conversioni sono non bloccanti: il programma
 * le avvia, continua a gestire display e pulsante e legge il risultato dopo 750 ms.
 */

void initializeTemperatureSensorBus() {
    ds18b20TemperatureSensors.begin();
    discoverTemperatureProbes();
    ds18b20TemperatureSensors.setResolution(DS18B20_RESOLUTION_BITS); // Imposta la precisione della misura e il relativo tempo di conversione.
    ds18b20TemperatureSensors.setWaitForConversion(false); // Permette al programma di continuare mentre le sonde stanno misurando.

    if (activeTemperatureProbeCount > 0) {
        ds18b20TemperatureSensors.requestTemperatures(); // Avvia la prima misura per tutte le sonde, senza attenderne qui il completamento.
        lastTemperatureConversionStartMilliseconds = millis(); // Da questo istante decorre l'attesa della conversione.
        temperatureConversionIsPending = true; // Il ciclo principale leggera i risultati quando saranno pronti.
    }
}

void discoverTemperatureProbes() {
    DeviceAddress discoveredRomAddress; // Buffer temporaneo in cui la libreria copia l'indirizzo ROM trovato.
    uint8_t recognizedDs18b20ProbeCount = 0; // Conta le DS18B20 trovate, incluse quelle oltre il limite supportato.
    const uint8_t oneWireDeviceCount = ds18b20TemperatureSensors.getDeviceCount(); // Numero di dispositivi rilevati sul bus OneWire.

    // La ricerca avviene solo all'avvio: dopo aver aggiunto o rimosso sonde occorre riavviare.
    for (uint8_t deviceIndex = 0; deviceIndex < oneWireDeviceCount; ++deviceIndex) { // Indice assegnato dalla libreria al dispositivo.
        if (!ds18b20TemperatureSensors.getAddress(discoveredRomAddress, deviceIndex) ||
            discoveredRomAddress[0] != DS18B20_FAMILY_CODE) {
            continue; // Salta dispositivi non leggibili o che non appartengono alla famiglia DS18B20.
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
}

void updateTemperatureMeasurements(uint32_t currentTimeMilliseconds) {
    if (activeTemperatureProbeCount == 0) {
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
            temperatureProbes[probeIndex].temperatureHistoryCelsius[nextTemperatureHistoryWriteIndex] =
                measuredTemperatureCelsius; // Tutte le sonde usano la stessa posizione per restare allineate nel tempo.
        }

        // Avanza una sola volta dopo aver salvato tutte le sonde, cosi il campione temporale resta comune.
        nextTemperatureHistoryWriteIndex =
            (nextTemperatureHistoryWriteIndex + 1) % TEMPERATURE_HISTORY_CAPACITY;
        if (storedTemperatureHistorySampleCount < TEMPERATURE_HISTORY_CAPACITY) {
            ++storedTemperatureHistorySampleCount; // Prima del riempimento, indica quante posizioni contengono gia un campione.
        }

        temperatureConversionIsPending = false;
        temperatureDisplayNeedsRedraw = true;
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
