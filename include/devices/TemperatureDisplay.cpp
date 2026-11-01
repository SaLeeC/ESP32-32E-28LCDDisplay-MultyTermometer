/*
 * Modulo del display TFT e delle due viste disponibili.
 *
 * La vista numerica rende immediato il controllo delle letture correnti.
 * La vista grafica usa uno storico circolare: il campione piu recente appare
 * a destra e una lettura non valida interrompe la curva, invece di nascondere
 * il problema collegando punti che non sono stati realmente misurati.
 */

#include <PNGdec.h>
#include <new>

#include "SondArisPngData.h"

static PNG* sondarisStartupLogoPngDecoder = nullptr; // Decoder temporaneo, liberato dopo aver disegnato il logo.

static void drawFirmwareVersionLabel(int16_t verticalPositionPixels) {
    char firmwareVersionText[FIRMWARE_VERSION_TEXT_BUFFER_CAPACITY]; // Buffer temporaneo per la versione mostrata all'operatore.
    snprintf(firmwareVersionText,
             sizeof(firmwareVersionText),
             FIRMWARE_VERSION_DISPLAY_FORMAT,
             static_cast<unsigned int>(FIRMWARE_VERSION_FIRST_NUMBER),
             static_cast<unsigned int>(FIRMWARE_VERSION_SECOND_NUMBER),
             FIRMWARE_VERSION_RELEASE_DATE);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    temperatureDisplay.drawRightString(
        firmwareVersionText,
        temperatureDisplay.width() - 8,
        verticalPositionPixels,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

/*
 * PNGdec restituisce un'immagine per righe; ogni riga viene convertita in
 * RGB565 e inviata subito al TFT senza riservare memoria per l'intero schermo.
 */
static int drawSondArisLogoPngLine(PNGDRAW* logoPngLine) {
    uint16_t decodedLogoPixelLine[SONDARIS_LOGO_IMAGE_WIDTH_PIXELS]; // Buffer temporaneo per una sola riga RGB565.
    sondarisStartupLogoPngDecoder->getLineAsRGB565(
        logoPngLine,
        decodedLogoPixelLine,
        PNG_RGB565_BIG_ENDIAN,
        0xFFFFFFFFUL);
    temperatureDisplay.pushImage(
        0,
        logoPngLine->y,
        logoPngLine->iWidth,
        1,
        decodedLogoPixelLine);
    return 1; // Comunica a PNGdec che la riga e stata elaborata correttamente.
}

static void releaseSondArisLogoPngDecoder() {
    if (sondarisStartupLogoPngDecoder == nullptr) {
        return;
    }

    sondarisStartupLogoPngDecoder->close();
    delete sondarisStartupLogoPngDecoder;
    sondarisStartupLogoPngDecoder = nullptr;
}

/*
 * Mostra il PNG incorporato nel firmware. Il controllo delle dimensioni
 * protegge il buffer di riga se in futuro viene sostituita l'immagine.
 */
static bool drawSondArisLogoFromPng() {
    sondarisStartupLogoPngDecoder = new (std::nothrow) PNG();
    if (sondarisStartupLogoPngDecoder == nullptr) {
        Serial.println("Memoria insufficiente per decodificare il logo PNG.");
        return false;
    }

    // PNGdec legge i dati in flash, ma la sua interfaccia non dichiara const.
    const int pngOpenResult = sondarisStartupLogoPngDecoder->openFLASH(
        const_cast<uint8_t*>(sondarisLogoPngData),
        static_cast<int>(sizeof(sondarisLogoPngData)),
        drawSondArisLogoPngLine);

    if (pngOpenResult != PNG_SUCCESS) {
        Serial.printf("Errore apertura logo PNG: codice %d.\n", pngOpenResult);
        releaseSondArisLogoPngDecoder();
        return false;
    }

    if (sondarisStartupLogoPngDecoder->getWidth() != SONDARIS_LOGO_IMAGE_WIDTH_PIXELS ||
        sondarisStartupLogoPngDecoder->getHeight() != SONDARIS_LOGO_IMAGE_HEIGHT_PIXELS) {
        Serial.printf(
            "Dimensioni logo PNG inattese: %d x %d pixel.\n",
            sondarisStartupLogoPngDecoder->getWidth(),
            sondarisStartupLogoPngDecoder->getHeight());
        releaseSondArisLogoPngDecoder();
        return false;
    }

    temperatureDisplay.startWrite();
    const int pngDecodeResult = sondarisStartupLogoPngDecoder->decode(nullptr, 0);
    temperatureDisplay.endWrite();
    releaseSondArisLogoPngDecoder();

    if (pngDecodeResult != PNG_SUCCESS) {
        Serial.printf("Errore decodifica logo PNG: codice %d.\n", pngDecodeResult);
        return false;
    }

    return true;
}

/*
 * Disegna un simbolo originale: una sfera osservata attraverso orbite
 * ellittiche, con un piccolo punto caldo che richiama una sonda in ascolto.
 */
static void drawSondArisLogoEmblem(int16_t screenCenterX) {
    const int16_t emblemCenterY = SONDARIS_LOGO_EMBLEM_CENTER_Y_PIXELS; // Centro del disegno nella parte alta dello schermo.

    // Pochi punti luminosi danno profondita senza affollare la schermata.
    temperatureDisplay.drawPixel(screenCenterX - 112, emblemCenterY - 17, TFT_LIGHTGREY);
    temperatureDisplay.drawPixel(screenCenterX + 117, emblemCenterY + 13, TFT_LIGHTGREY);
    temperatureDisplay.drawPixel(screenCenterX - 96, emblemCenterY + 34, SONDARIS_LOGO_ACCENT_COLOR);
    temperatureDisplay.drawPixel(screenCenterX + 102, emblemCenterY - 33, SONDARIS_LOGO_INNER_ORBIT_COLOR);

    // Le orbite sono disegnate per prime: la sfera copre il tratto che passa dietro al pianeta.
    temperatureDisplay.drawEllipse(
        screenCenterX,
        emblemCenterY,
        SONDARIS_LOGO_OUTER_ORBIT_RADIUS_X_PIXELS,
        SONDARIS_LOGO_OUTER_ORBIT_RADIUS_Y_PIXELS,
        SONDARIS_LOGO_OUTER_ORBIT_COLOR);
    temperatureDisplay.drawEllipse(
        screenCenterX,
        emblemCenterY,
        SONDARIS_LOGO_INNER_ORBIT_RADIUS_X_PIXELS,
        SONDARIS_LOGO_INNER_ORBIT_RADIUS_Y_PIXELS,
        SONDARIS_LOGO_INNER_ORBIT_COLOR);

    // Due toni sovrapposti suggeriscono la luce sulla superficie sferica.
    temperatureDisplay.fillCircle(
        screenCenterX,
        emblemCenterY,
        SONDARIS_LOGO_PLANET_RADIUS_PIXELS,
        SONDARIS_LOGO_PLANET_EDGE_COLOR);
    temperatureDisplay.fillCircle(
        screenCenterX + 6,
        emblemCenterY - 2,
        SONDARIS_LOGO_PLANET_RADIUS_PIXELS - 5,
        SONDARIS_LOGO_PLANET_COLOR);
    temperatureDisplay.drawCircle(
        screenCenterX,
        emblemCenterY,
        SONDARIS_LOGO_PLANET_RADIUS_PIXELS,
        SONDARIS_LOGO_PLANET_EDGE_COLOR);

    // Il punto arancione e la linea breve richiamano una sonda e un orizzonte lontano.
    temperatureDisplay.fillCircle(screenCenterX, emblemCenterY - 7, 4, SONDARIS_LOGO_ACCENT_COLOR);
    temperatureDisplay.drawLine(
        screenCenterX - 15,
        emblemCenterY + 9,
        screenCenterX + 16,
        emblemCenterY + 9,
        TFT_LIGHTGREY);
}

void initializeTemperatureDisplay() {
    temperatureDisplay.init();
    temperatureDisplay.setRotation(TEMPERATURE_DISPLAY_LANDSCAPE_ROTATION); // Usa l'orientamento orizzontale previsto per la scheda.
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Accende la retroilluminazione configurata in platformio.ini.
}

void drawStartupPresentation() {
    const int16_t screenCenterX = temperatureDisplay.width() / 2; // Punto centrale orizzontale usato per allineare i testi.
    temperatureDisplay.fillScreen(TFT_BLACK);

    if (!drawSondArisLogoFromPng()) {
        // Se il PNG non e leggibile, il simbolo vettoriale resta disponibile e il problema e gia segnalato in seriale.
        temperatureDisplay.fillScreen(TFT_BLACK);
        drawSondArisLogoEmblem(screenCenterX);
        temperatureDisplay.setTextFont(SONDARIS_LOGO_WORDMARK_FONT);
        temperatureDisplay.setTextColor(SONDARIS_LOGO_WORDMARK_COLOR, TFT_BLACK);
        temperatureDisplay.drawCentreString(
            SONDARIS_LOGO_WORDMARK,
            screenCenterX,
            SONDARIS_LOGO_WORDMARK_Y_PIXELS,
            SONDARIS_LOGO_WORDMARK_FONT);
    }

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(SONDARIS_LOGO_DESCRIPTION_COLOR, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        SONDARIS_LOGO_DESCRIPTION,
        screenCenterX,
        SONDARIS_LOGO_DESCRIPTION_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);

    drawFirmwareVersionLabel(STARTUP_PRESENTATION_VERSION_Y_PIXELS);
}

static uint64_t convertBytesToTenthsOfDisplayUnit(
    uint64_t byteCount,
    uint64_t bytesPerDisplayUnit) {
    const uint64_t wholeDisplayUnits = byteCount / bytesPerDisplayUnit;
    const uint64_t remainingBytes = byteCount % bytesPerDisplayUnit;
    uint64_t fractionalDisplayUnitTenths =
        (remainingBytes * 10ULL + bytesPerDisplayUnit / 2ULL) / bytesPerDisplayUnit;

    if (fractionalDisplayUnitTenths == 10ULL) {
        return (wholeDisplayUnits + 1ULL) * 10ULL;
    }

    return wholeDisplayUnits * 10ULL + fractionalDisplayUnitTenths;
}

/*
 * Formatta capacita totale e spazio residuo con la stessa unita, scelta
 * in base alla capacita del filesystem.
 */
static void formatMicroSdCapacityAndFreeSpace(
    const MicroSdCardCheckResult& microSdCardCheckResult,
    bool compactForWarning,
    char* formattedSpaceSummary,
    size_t formattedSpaceSummaryCapacity) {
    uint64_t bytesPerDisplayUnit = 1;
    const char* displayUnitLabel = "B";

    if (microSdCardCheckResult.totalCapacityBytes >= MICRO_SD_BYTES_PER_TERABYTE) {
        bytesPerDisplayUnit = MICRO_SD_BYTES_PER_TERABYTE;
        displayUnitLabel = "TB";
    } else if (microSdCardCheckResult.totalCapacityBytes >= MICRO_SD_BYTES_PER_GIGABYTE) {
        bytesPerDisplayUnit = MICRO_SD_BYTES_PER_GIGABYTE;
        displayUnitLabel = "GB";
    } else if (microSdCardCheckResult.totalCapacityBytes >= MICRO_SD_BYTES_PER_MEGABYTE) {
        bytesPerDisplayUnit = MICRO_SD_BYTES_PER_MEGABYTE;
        displayUnitLabel = "MB";
    } else if (microSdCardCheckResult.totalCapacityBytes >= MICRO_SD_BYTES_PER_KILOBYTE) {
        bytesPerDisplayUnit = MICRO_SD_BYTES_PER_KILOBYTE;
        displayUnitLabel = "KB";
    }

    const uint64_t totalCapacityTenths =
        convertBytesToTenthsOfDisplayUnit(
            microSdCardCheckResult.totalCapacityBytes,
            bytesPerDisplayUnit);
    const uint64_t freeSpaceTenths =
        convertBytesToTenthsOfDisplayUnit(
            microSdCardCheckResult.freeSpaceBytes,
            bytesPerDisplayUnit);

    if (compactForWarning) {
        const uint64_t roundedTotalCapacityUnits =
            (microSdCardCheckResult.totalCapacityBytes + bytesPerDisplayUnit / 2ULL) /
            bytesPerDisplayUnit;
        snprintf(
            formattedSpaceSummary,
            formattedSpaceSummaryCapacity,
            "%llu - %llu.%llu %s",
            static_cast<unsigned long long>(roundedTotalCapacityUnits),
            static_cast<unsigned long long>(freeSpaceTenths / 10ULL),
            static_cast<unsigned long long>(freeSpaceTenths % 10ULL),
            displayUnitLabel);
        return;
    }

    snprintf(
        formattedSpaceSummary,
        formattedSpaceSummaryCapacity,
        "%llu.%llu - %llu.%llu %s",
        static_cast<unsigned long long>(totalCapacityTenths / 10ULL),
        static_cast<unsigned long long>(totalCapacityTenths % 10ULL),
        static_cast<unsigned long long>(freeSpaceTenths / 10ULL),
        static_cast<unsigned long long>(freeSpaceTenths % 10ULL),
        displayUnitLabel);
}

static void formatExternalSensorBusStatus(
    const ExternalSensorBusCheckResult& externalSensorBusCheckResult,
    char* formattedBusStatus,
    size_t formattedBusStatusCapacity) {
    switch (externalSensorBusCheckResult.mode) {
        case ExternalSensorBusMode::OneWire:
            snprintf(
                formattedBusStatus,
                formattedBusStatusCapacity,
                TECHNICAL_MENU_SENSOR_BUS_ONEWIRE_FORMAT,
                static_cast<unsigned int>(
                    externalSensorBusCheckResult.oneWireDeviceCount),
                static_cast<unsigned int>(getExternalSensorSingleWireDataPin(
                    externalSensorBusCheckResult.pinMapping)));
            break;
        case ExternalSensorBusMode::Dht:
            if (externalSensorBusCheckResult.dhtModel ==
                DhtSensorModel::NotSelected) {
                snprintf(
                    formattedBusStatus,
                    formattedBusStatusCapacity,
                    EXTERNAL_SENSOR_BUS_DHT_PENDING_FORMAT,
                    static_cast<unsigned int>(getExternalSensorSingleWireDataPin(
                        externalSensorBusCheckResult.pinMapping)));
            } else {
                snprintf(
                    formattedBusStatus,
                    formattedBusStatusCapacity,
                    EXTERNAL_SENSOR_BUS_DHT_SELECTED_FORMAT,
                    getDhtSensorShortModelName(
                        externalSensorBusCheckResult.dhtModel),
                    static_cast<unsigned int>(getExternalSensorSingleWireDataPin(
                        externalSensorBusCheckResult.pinMapping)));
            }
            break;
        case ExternalSensorBusMode::I2c: {
            int formattedBusStatusLength = snprintf(
                formattedBusStatus,
                formattedBusStatusCapacity,
                TECHNICAL_MENU_SENSOR_BUS_I2C_FORMAT,
                static_cast<unsigned int>(
                    externalSensorBusCheckResult.i2cDeviceCount),
                static_cast<unsigned int>(getExternalSensorI2cSdaPin(
                    externalSensorBusCheckResult.pinMapping)),
                static_cast<unsigned int>(getExternalSensorI2cSclPin(
                    externalSensorBusCheckResult.pinMapping)));
            if (formattedBusStatusLength < 0 ||
                static_cast<size_t>(formattedBusStatusLength) >=
                    formattedBusStatusCapacity) {
                formattedBusStatus[formattedBusStatusCapacity - 1] = '\0';
                return;
            }

            size_t formattedBusStatusUsedLength =
                static_cast<size_t>(formattedBusStatusLength);
            uint8_t displayedStartupI2cAddressCount = 0;
            while (displayedStartupI2cAddressCount <
                       externalSensorBusCheckResult.i2cDeviceCount &&
                   displayedStartupI2cAddressCount <
                       STARTUP_SUMMARY_I2C_ADDRESSES_TO_DISPLAY) {
                const size_t remainingBusStatusCapacity =
                    formattedBusStatusCapacity - formattedBusStatusUsedLength;
                const int appendedAddressLength = snprintf(
                    formattedBusStatus + formattedBusStatusUsedLength,
                    remainingBusStatusCapacity,
                    " 0x%02X",
                    static_cast<unsigned int>(
                        externalSensorBusCheckResult.i2cDeviceAddresses[
                            displayedStartupI2cAddressCount]));
                if (appendedAddressLength < 0 ||
                    static_cast<size_t>(appendedAddressLength) >=
                        remainingBusStatusCapacity) {
                    formattedBusStatus[formattedBusStatusCapacity - 1] = '\0';
                    return;
                }

                formattedBusStatusUsedLength +=
                    static_cast<size_t>(appendedAddressLength);
                ++displayedStartupI2cAddressCount;
            }

            if (displayedStartupI2cAddressCount <
                externalSensorBusCheckResult.i2cDeviceCount) {
                const size_t remainingBusStatusCapacity =
                    formattedBusStatusCapacity - formattedBusStatusUsedLength;
                if (remainingBusStatusCapacity > 2) {
                    snprintf(
                        formattedBusStatus + formattedBusStatusUsedLength,
                        remainingBusStatusCapacity,
                        " +");
                }
            }
            break;
        }
        case ExternalSensorBusMode::I2cInitializationError:
            snprintf(
                formattedBusStatus,
                formattedBusStatusCapacity,
                "%s",
                TECHNICAL_MENU_SENSOR_BUS_ERROR_VALUE);
            break;
        case ExternalSensorBusMode::NoSensorDetected:
            snprintf(
                formattedBusStatus,
                formattedBusStatusCapacity,
                "%s",
                TECHNICAL_MENU_SENSOR_BUS_NO_SENSOR_VALUE);
            break;
        case ExternalSensorBusMode::NotChecked:
            snprintf(
                formattedBusStatus,
                formattedBusStatusCapacity,
                "%s",
                TECHNICAL_MENU_SENSOR_BUS_NOT_CHECKED_VALUE);
            break;
    }
}

static void formatExternalI2cAddressList(
    const ExternalSensorBusCheckResult& externalSensorBusCheckResult,
    char* formattedI2cAddressList,
    size_t formattedI2cAddressListCapacity) {
    const char* unavailableAddressListText =
        TECHNICAL_MENU_I2C_ADDRESSES_NOT_SCANNED_VALUE;

    switch (externalSensorBusCheckResult.mode) {
        case ExternalSensorBusMode::OneWire:
            unavailableAddressListText =
                TECHNICAL_MENU_I2C_ADDRESSES_NOT_SCANNED_VALUE;
            break;
        case ExternalSensorBusMode::Dht:
            unavailableAddressListText =
                TECHNICAL_MENU_I2C_ADDRESSES_NOT_SCANNED_VALUE;
            break;
        case ExternalSensorBusMode::I2cInitializationError:
            unavailableAddressListText =
                TECHNICAL_MENU_I2C_ADDRESSES_ERROR_VALUE;
            break;
        case ExternalSensorBusMode::NoSensorDetected:
            unavailableAddressListText =
                TECHNICAL_MENU_I2C_ADDRESSES_NONE_VALUE;
            break;
        case ExternalSensorBusMode::NotChecked:
            unavailableAddressListText =
                TECHNICAL_MENU_SENSOR_BUS_NOT_CHECKED_VALUE;
            break;
        case ExternalSensorBusMode::I2c:
            if (externalSensorBusCheckResult.i2cDeviceCount == 0) {
                unavailableAddressListText =
                    TECHNICAL_MENU_I2C_ADDRESSES_NONE_VALUE;
                break;
            }

            formattedI2cAddressList[0] = '\0';
            size_t formattedAddressListLength = 0;
            uint8_t displayedAddressCount = 0;
            while (displayedAddressCount <
                       externalSensorBusCheckResult.i2cDeviceCount &&
                   displayedAddressCount <
                       TECHNICAL_MENU_I2C_ADDRESSES_TO_DISPLAY) {
                const size_t remainingBufferCapacity =
                    formattedI2cAddressListCapacity -
                    formattedAddressListLength;
                const int appendedAddressLength = snprintf(
                    formattedI2cAddressList + formattedAddressListLength,
                    remainingBufferCapacity,
                    displayedAddressCount == 0 ? "0x%02X" : " 0x%02X",
                    static_cast<unsigned int>(
                        externalSensorBusCheckResult.i2cDeviceAddresses[
                            displayedAddressCount]));
                if (appendedAddressLength < 0 ||
                    static_cast<size_t>(appendedAddressLength) >=
                        remainingBufferCapacity) {
                    formattedI2cAddressList[
                        formattedI2cAddressListCapacity - 1] = '\0';
                    return;
                }

                formattedAddressListLength +=
                    static_cast<size_t>(appendedAddressLength);
                ++displayedAddressCount;
            }

            if (displayedAddressCount <
                externalSensorBusCheckResult.i2cDeviceCount) {
                const size_t remainingBufferCapacity =
                    formattedI2cAddressListCapacity -
                    formattedAddressListLength;
                if (remainingBufferCapacity > 2) {
                    snprintf(
                        formattedI2cAddressList + formattedAddressListLength,
                        remainingBufferCapacity,
                        " +");
                }
            }
            return;
    }

    snprintf(
        formattedI2cAddressList,
        formattedI2cAddressListCapacity,
        "%s",
        unavailableAddressListText);
}

void drawStartupSummaryScreen(
    const MicroSdCardCheckResult& microSdCardCheckResult,
    const ExternalSensorBusCheckResult& externalSensorBusCheckResult) {
    const int16_t screenCenterX = temperatureDisplay.width() / 2; // Punto centrale usato per allineare titolo e note.
    const int16_t microSdStatusCellCenterX =
        (STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS +
         STARTUP_SUMMARY_TABLE_X_PIXELS +
         STARTUP_SUMMARY_TABLE_WIDTH_PIXELS) / 2; // Centra errori e avvisi nella cella del risultato.
    const int16_t sensorBusStatusCellCenterX = microSdStatusCellCenterX; // Le celle hardware condividono la stessa colonna dei risultati.
    const bool externalSensorBusFoundDevice =
        externalSensorBusCheckResult.mode == ExternalSensorBusMode::OneWire ||
        externalSensorBusCheckResult.mode == ExternalSensorBusMode::Dht ||
        (externalSensorBusCheckResult.mode == ExternalSensorBusMode::I2c &&
         externalSensorBusCheckResult.i2cDeviceCount > 0);
    const bool externalSensorBusHasError =
        externalSensorBusCheckResult.mode ==
        ExternalSensorBusMode::I2cInitializationError;
    const bool sensorBusStatusCellNeedsBackground =
        externalSensorBusFoundDevice || externalSensorBusHasError;
    const uint16_t sensorBusStatusCellBackgroundColor =
        externalSensorBusHasError ? TFT_RED : TFT_DARKGREEN;
    const uint16_t sensorBusStatusTextColor =
        sensorBusStatusCellNeedsBackground ? TFT_WHITE : TFT_ORANGE;
    char externalSensorBusStatusText[STARTUP_SUMMARY_SENSOR_BUS_TEXT_BUFFER_CAPACITY];
    formatExternalSensorBusStatus(
        externalSensorBusCheckResult,
        externalSensorBusStatusText,
        sizeof(externalSensorBusStatusText));
    const bool microSdFileSystemIsReady =
        microSdCardCheckResult.status == MicroSdCardCheckStatus::Ready;
    const bool microSdFileSystemHasError =
        microSdCardCheckResult.status == MicroSdCardCheckStatus::FileSystemError;
    const bool microSdFreeSpaceIsLow =
        microSdFileSystemIsReady &&
        microSdCardCheckResult.freeSpaceBytes < MICRO_SD_LOW_FREE_SPACE_LIMIT_BYTES;
    const bool microSdStatusCellNeedsRedBackground =
        microSdFileSystemHasError || microSdFreeSpaceIsLow;
    const bool microSdStatusCellNeedsGreenBackground =
        microSdFileSystemIsReady && !microSdFreeSpaceIsLow;
    const bool microSdStatusCellNeedsBackground =
        microSdStatusCellNeedsRedBackground || microSdStatusCellNeedsGreenBackground;
    const uint16_t microSdStatusCellBackgroundColor =
        microSdStatusCellNeedsRedBackground
            ? TFT_RED
            : (microSdStatusCellNeedsGreenBackground ? TFT_DARKGREEN : TFT_BLACK);
    const bool microSdStatusTextNeedsEmphasis = microSdStatusCellNeedsRedBackground;
    const bool microSdDataDirectoryWasCreated =
        microSdCardCheckResult.dataDirectoryStatus == MicroSdDataDirectoryStatus::Created;
    const bool microSdDataDirectoryHasError =
        microSdFileSystemIsReady &&
        microSdCardCheckResult.dataDirectoryStatus != MicroSdDataDirectoryStatus::Created &&
        microSdCardCheckResult.dataDirectoryStatus != MicroSdDataDirectoryStatus::AlreadyPresent;
    char microSdCapacityAndFreeSpaceText[STARTUP_SUMMARY_STORAGE_TEXT_BUFFER_CAPACITY]; // Testo compatto con capacita e spazio residuo nella stessa unita.
    const char* microSdStatusText = STARTUP_SUMMARY_MICRO_SD_NOT_DETECTED_VALUE;
    const char* microSdStatusDetail = STARTUP_SUMMARY_MICRO_SD_NOT_DETECTED_DETAIL;
    uint16_t microSdStatusTextColor = TFT_ORANGE;
    uint16_t microSdStatusDetailColor = TFT_LIGHTGREY;

    if (microSdFileSystemIsReady) {
        formatMicroSdCapacityAndFreeSpace(
            microSdCardCheckResult,
            microSdFreeSpaceIsLow,
            microSdCapacityAndFreeSpaceText,
            sizeof(microSdCapacityAndFreeSpaceText));
        microSdStatusText = microSdCapacityAndFreeSpaceText;
        if (microSdCardCheckResult.dataDirectoryStatus ==
            MicroSdDataDirectoryStatus::AlreadyPresent) {
            microSdStatusDetail = STARTUP_SUMMARY_MICRO_SD_DIRECTORY_PRESENT_DETAIL;
        } else if (microSdDataDirectoryWasCreated) {
            microSdStatusDetail = STARTUP_SUMMARY_MICRO_SD_DIRECTORY_CREATED_DETAIL;
        } else {
            microSdStatusDetail = STARTUP_SUMMARY_MICRO_SD_DIRECTORY_ERROR_DETAIL;
        }
        microSdStatusTextColor = TFT_WHITE;
    } else if (microSdFileSystemHasError) {
        microSdStatusText = STARTUP_SUMMARY_MICRO_SD_FILESYSTEM_ERROR_VALUE;
        microSdStatusDetail = STARTUP_SUMMARY_MICRO_SD_FILESYSTEM_ERROR_DETAIL;
        microSdStatusTextColor = TFT_WHITE;
    }

    if (microSdFileSystemHasError || microSdFreeSpaceIsLow || microSdDataDirectoryHasError) {
        microSdStatusDetailColor = TFT_ORANGE;
    } else if (microSdDataDirectoryWasCreated) {
        microSdStatusDetailColor = TFT_GREEN;
    }

    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        STARTUP_SUMMARY_SCREEN_TITLE,
        screenCenterX,
        STARTUP_SUMMARY_TITLE_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    if (sensorBusStatusCellNeedsBackground) {
        temperatureDisplay.fillRect(
            STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS + 1,
            STARTUP_SUMMARY_TABLE_Y_PIXELS +
                STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS *
                    STARTUP_SUMMARY_SENSOR_BUS_ROW_INDEX + 1,
            STARTUP_SUMMARY_TABLE_X_PIXELS +
                STARTUP_SUMMARY_TABLE_WIDTH_PIXELS -
                STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS - 2,
            STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS - 2,
            sensorBusStatusCellBackgroundColor);
    }

    if (microSdStatusCellNeedsBackground) {
        temperatureDisplay.fillRect(
            STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS + 1,
            STARTUP_SUMMARY_TABLE_Y_PIXELS +
                STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS *
                    STARTUP_SUMMARY_HARDWARE_ROW_INDEX + 1,
            STARTUP_SUMMARY_TABLE_X_PIXELS +
                STARTUP_SUMMARY_TABLE_WIDTH_PIXELS -
                STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS - 2,
            STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS - 2,
            microSdStatusCellBackgroundColor);
    }

    temperatureDisplay.drawRect(
        STARTUP_SUMMARY_TABLE_X_PIXELS,
        STARTUP_SUMMARY_TABLE_Y_PIXELS,
        STARTUP_SUMMARY_TABLE_WIDTH_PIXELS,
        STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS *
            STARTUP_SUMMARY_TABLE_ROW_COUNT,
        TFT_DARKCYAN);
    temperatureDisplay.drawFastVLine(
        STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS,
        STARTUP_SUMMARY_TABLE_Y_PIXELS,
        STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS *
            STARTUP_SUMMARY_TABLE_ROW_COUNT,
        TFT_DARKCYAN);
    for (uint8_t rowIndex = 1;
         rowIndex < STARTUP_SUMMARY_TABLE_ROW_COUNT;
         ++rowIndex) {
        temperatureDisplay.drawFastHLine(
            STARTUP_SUMMARY_TABLE_X_PIXELS,
            STARTUP_SUMMARY_TABLE_Y_PIXELS +
                STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS * rowIndex,
            STARTUP_SUMMARY_TABLE_WIDTH_PIXELS,
            TFT_DARKCYAN);
    }

    const int16_t firstSummaryRowTextY = STARTUP_SUMMARY_FIRST_ROW_TEXT_Y_PIXELS; // Posizione verticale della riga con il nome del progetto.
    const int16_t secondSummaryRowTextY =
        firstSummaryRowTextY + STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS; // Posizione verticale della riga con la versione firmware.
    const int16_t sensorBusSummaryRowTextY =
        firstSummaryRowTextY +
        STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS *
            STARTUP_SUMMARY_SENSOR_BUS_ROW_INDEX; // Posizione verticale della riga con il risultato OneWire/I2C.
    const int16_t hardwareSummaryRowTextY =
        firstSummaryRowTextY +
        STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS *
            STARTUP_SUMMARY_HARDWARE_ROW_INDEX; // Posizione verticale della riga con il risultato microSD.

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    temperatureDisplay.drawString(
        STARTUP_SUMMARY_NAME_LABEL,
        STARTUP_SUMMARY_LABEL_X_PIXELS,
        firstSummaryRowTextY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawString(
        STARTUP_SUMMARY_VERSION_LABEL,
        STARTUP_SUMMARY_LABEL_X_PIXELS,
        secondSummaryRowTextY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawString(
        STARTUP_SUMMARY_SENSOR_BUS_LABEL,
        STARTUP_SUMMARY_LABEL_X_PIXELS,
        sensorBusSummaryRowTextY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawString(
        STARTUP_SUMMARY_HARDWARE_LABEL,
        STARTUP_SUMMARY_LABEL_X_PIXELS,
        hardwareSummaryRowTextY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);

    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.drawString(
        SONDARIS_LOGO_WORDMARK,
        STARTUP_SUMMARY_VALUE_X_PIXELS,
        firstSummaryRowTextY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    drawFirmwareVersionLabel(secondSummaryRowTextY);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextSize(externalSensorBusHasError ? 2 : 1);
    if (sensorBusStatusCellNeedsBackground) {
        temperatureDisplay.setTextColor(sensorBusStatusTextColor);
        temperatureDisplay.drawCentreString(
            externalSensorBusStatusText,
            sensorBusStatusCellCenterX,
            sensorBusSummaryRowTextY,
            TEMPERATURE_DISPLAY_GRAPH_FONT);
    } else {
        temperatureDisplay.setTextColor(sensorBusStatusTextColor, TFT_BLACK);
        temperatureDisplay.drawString(
            externalSensorBusStatusText,
            STARTUP_SUMMARY_VALUE_X_PIXELS,
            sensorBusSummaryRowTextY,
            TEMPERATURE_DISPLAY_GRAPH_FONT);
    }
    temperatureDisplay.setTextSize(1);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextSize(microSdStatusTextNeedsEmphasis ? 2 : 1);
    if (microSdStatusCellNeedsBackground) {
        temperatureDisplay.setTextColor(microSdStatusTextColor);
        temperatureDisplay.drawCentreString(
            microSdStatusText,
            microSdStatusCellCenterX,
            hardwareSummaryRowTextY,
            TEMPERATURE_DISPLAY_GRAPH_FONT);
        if (microSdStatusTextNeedsEmphasis) {
            temperatureDisplay.drawCentreString(
                microSdStatusText,
                microSdStatusCellCenterX + STARTUP_SUMMARY_BOLD_TEXT_OFFSET_PIXELS,
                hardwareSummaryRowTextY,
                TEMPERATURE_DISPLAY_GRAPH_FONT);
        }
    } else {
        temperatureDisplay.setTextColor(microSdStatusTextColor, TFT_BLACK);
        temperatureDisplay.drawString(
            microSdStatusText,
            STARTUP_SUMMARY_VALUE_X_PIXELS,
            hardwareSummaryRowTextY,
            TEMPERATURE_DISPLAY_GRAPH_FONT);
    }
    temperatureDisplay.setTextSize(1);

    temperatureDisplay.setTextColor(microSdStatusDetailColor, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        microSdStatusDetail,
        screenCenterX,
        STARTUP_SUMMARY_STATUS_DETAIL_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);

    temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        STARTUP_SUMMARY_NO_DATA_FILES_NOTE,
        screenCenterX,
        STARTUP_SUMMARY_NO_DATA_FILES_NOTE_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_SUMMARY_NEXT_STEP_NOTE,
        screenCenterX,
        STARTUP_SUMMARY_NEXT_STEP_NOTE_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

/*
 * Disegna la cornice e l'icona della scheda di attesa dei sensori.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: pulisce il display e disegna lo sfondo decorativo della scheda.
 */
static void drawStartupNoSensorCardFrame() {
    const int16_t iconCenterX = STARTUP_NO_SENSOR_ICON_CENTER_X_PIXELS;
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.fillRoundRect(
        STARTUP_NO_SENSOR_CARD_X_PIXELS,
        STARTUP_NO_SENSOR_CARD_Y_PIXELS,
        STARTUP_NO_SENSOR_CARD_WIDTH_PIXELS,
        STARTUP_NO_SENSOR_CARD_HEIGHT_PIXELS,
        10,
        STARTUP_NO_SENSOR_CARD_COLOR);
    temperatureDisplay.drawRoundRect(
        STARTUP_NO_SENSOR_CARD_X_PIXELS,
        STARTUP_NO_SENSOR_CARD_Y_PIXELS,
        STARTUP_NO_SENSOR_CARD_WIDTH_PIXELS,
        STARTUP_NO_SENSOR_CARD_HEIGHT_PIXELS,
        10,
        STARTUP_NO_SENSOR_BORDER_COLOR);
    temperatureDisplay.drawCircle(
        iconCenterX - 3,
        STARTUP_NO_SENSOR_ICON_CENTER_Y_PIXELS - 3,
        11,
        STARTUP_NO_SENSOR_ACCENT_COLOR);
    temperatureDisplay.drawCircle(
        iconCenterX - 3,
        STARTUP_NO_SENSOR_ICON_CENTER_Y_PIXELS - 3,
        5,
        STARTUP_NO_SENSOR_ACCENT_COLOR);
    temperatureDisplay.drawLine(
        iconCenterX + 5,
        STARTUP_NO_SENSOR_ICON_CENTER_Y_PIXELS + 5,
        iconCenterX + 14,
        STARTUP_NO_SENSOR_ICON_CENTER_Y_PIXELS + 14,
        STARTUP_NO_SENSOR_ACCENT_COLOR);
}

/*
 * Mostra il messaggio empatico e il pulsante che ripete la ricerca sensori.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna il display senza modificare lo stato del bus.
 */
void drawStartupNoSensorScreen() {
    const int16_t screenCenterX = temperatureDisplay.width() / 2;
    drawStartupNoSensorCardFrame();

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(
        STARTUP_NO_SENSOR_ACCENT_COLOR,
        STARTUP_NO_SENSOR_CARD_COLOR);
    temperatureDisplay.drawCentreString(
        STARTUP_NO_SENSOR_TITLE,
        screenCenterX,
        STARTUP_NO_SENSOR_TITLE_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.setTextColor(TFT_WHITE, STARTUP_NO_SENSOR_CARD_COLOR);
    temperatureDisplay.drawCentreString(
        STARTUP_NO_SENSOR_FIRST_INSTRUCTION,
        screenCenterX,
        STARTUP_NO_SENSOR_FIRST_INSTRUCTION_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_NO_SENSOR_SECOND_INSTRUCTION,
        screenCenterX,
        STARTUP_NO_SENSOR_SECOND_INSTRUCTION_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.fillRoundRect(
        STARTUP_NO_SENSOR_START_BUTTON_X_PIXELS,
        STARTUP_NO_SENSOR_START_BUTTON_Y_PIXELS,
        STARTUP_NO_SENSOR_START_BUTTON_WIDTH_PIXELS,
        STARTUP_NO_SENSOR_START_BUTTON_HEIGHT_PIXELS,
        8,
        STARTUP_NO_SENSOR_BUTTON_COLOR);
    temperatureDisplay.drawRoundRect(
        STARTUP_NO_SENSOR_START_BUTTON_X_PIXELS,
        STARTUP_NO_SENSOR_START_BUTTON_Y_PIXELS,
        STARTUP_NO_SENSOR_START_BUTTON_WIDTH_PIXELS,
        STARTUP_NO_SENSOR_START_BUTTON_HEIGHT_PIXELS,
        8,
        STARTUP_NO_SENSOR_BUTTON_BORDER_COLOR);
    temperatureDisplay.setTextColor(
        TFT_WHITE,
        STARTUP_NO_SENSOR_BUTTON_COLOR);
    temperatureDisplay.drawCentreString(
        STARTUP_NO_SENSOR_START_BUTTON_LABEL,
        screenCenterX,
        STARTUP_NO_SENSOR_START_BUTTON_Y_PIXELS + 11,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(
        TFT_LIGHTGREY,
        STARTUP_NO_SENSOR_CARD_COLOR);
    temperatureDisplay.drawCentreString(
        STARTUP_NO_SENSOR_BOOT_HINT,
        screenCenterX,
        STARTUP_NO_SENSOR_BOOT_HINT_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

/*
 * Mostra un feedback grafico mentre vengono ripetuti i test sui due GPIO.
 * Parametri: nessuno.
 * Restituisce: nulla.
 * Effetti: aggiorna il display senza modificare lo stato del bus.
 */
void drawStartupSensorSearchProgressScreen() {
    const int16_t screenCenterX = temperatureDisplay.width() / 2;
    drawStartupNoSensorCardFrame();
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(
        STARTUP_NO_SENSOR_ACCENT_COLOR,
        STARTUP_NO_SENSOR_CARD_COLOR);
    temperatureDisplay.drawCentreString(
        STARTUP_SENSOR_SEARCH_PROGRESS_MESSAGE,
        screenCenterX,
        STARTUP_NO_SENSOR_FIRST_INSTRUCTION_Y_PIXELS + 8,
        TEMPERATURE_DISPLAY_TITLE_FONT);
}

static void drawTouchCalibrationTargetScreen(
    uint8_t calibrationTargetIndex,
    const char* statusMessage,
    uint16_t statusMessageColor,
    uint16_t targetColor,
    uint16_t targetCenterColor,
    bool calibrationPointWasCaptured) {
    if (calibrationTargetIndex >= STARTUP_TOUCH_CALIBRATION_TARGET_COUNT) {
        return; // Un indice fuori intervallo non corrisponde a un bersaglio visibile.
    }

    char calibrationStepTitle[STARTUP_TOUCH_STEP_TITLE_BUFFER_CAPACITY]; // Buffer per il numero del punto da toccare.
    snprintf(calibrationStepTitle,
             sizeof(calibrationStepTitle),
             STARTUP_TOUCH_STEP_TITLE_FORMAT,
             static_cast<unsigned int>(calibrationTargetIndex + 1),
             static_cast<unsigned int>(STARTUP_TOUCH_CALIBRATION_TARGET_COUNT));

    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        calibrationStepTitle,
        temperatureDisplay.width() / 2,
        STARTUP_TOUCH_INSTRUCTION_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(statusMessageColor, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        statusMessage,
        temperatureDisplay.width() / 2,
        STARTUP_TOUCH_DETAIL_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);

    // Verde indica un punto acquisito, giallo/ciano/rosso il punto corrente e grigio quelli ancora da fare.
    for (uint8_t progressDotIndex = 0;
         progressDotIndex < STARTUP_TOUCH_CALIBRATION_TARGET_COUNT;
         ++progressDotIndex) {
        const int16_t progressDotXPositionPixels =
            temperatureDisplay.width() / 2 +
            (static_cast<int16_t>(progressDotIndex) -
             static_cast<int16_t>(STARTUP_TOUCH_CALIBRATION_TARGET_COUNT / 2)) * 20;
        const uint16_t progressDotColor = progressDotIndex < calibrationTargetIndex
            ? STARTUP_TOUCH_TARGET_CAPTURED_COLOR
            : (progressDotIndex == calibrationTargetIndex
                ? (calibrationPointWasCaptured
                    ? STARTUP_TOUCH_TARGET_CAPTURED_COLOR
                    : targetColor)
                : TFT_DARKGREY);
        temperatureDisplay.fillCircle(
            progressDotXPositionPixels,
            STARTUP_TOUCH_PROGRESS_Y_PIXELS,
            4,
            progressDotColor);
    }

    const int16_t targetXPositionPixels =
        STARTUP_TOUCH_CALIBRATION_TARGET_X_PIXELS[calibrationTargetIndex];
    const int16_t targetYPositionPixels =
        STARTUP_TOUCH_CALIBRATION_TARGET_Y_PIXELS[calibrationTargetIndex];
    temperatureDisplay.drawCircle(
        targetXPositionPixels,
        targetYPositionPixels,
        STARTUP_TOUCH_TARGET_RADIUS_PIXELS,
        targetColor);
    temperatureDisplay.drawFastHLine(
        targetXPositionPixels - STARTUP_TOUCH_TARGET_CROSSHAIR_HALF_SIZE_PIXELS,
        targetYPositionPixels,
        STARTUP_TOUCH_TARGET_CROSSHAIR_HALF_SIZE_PIXELS * 2 + 1,
        targetColor);
    temperatureDisplay.drawFastVLine(
        targetXPositionPixels,
        targetYPositionPixels - STARTUP_TOUCH_TARGET_CROSSHAIR_HALF_SIZE_PIXELS,
        STARTUP_TOUCH_TARGET_CROSSHAIR_HALF_SIZE_PIXELS * 2 + 1,
        targetColor);
    temperatureDisplay.fillCircle(
        targetXPositionPixels,
        targetYPositionPixels,
        3,
        targetCenterColor);

    temperatureDisplay.setTextColor(STARTUP_TOUCH_BOOT_HINT_COLOR, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_BOOT_SKIP_HINT,
        temperatureDisplay.width() / 2,
        STARTUP_TOUCH_BOOT_HINT_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

void drawTouchCalibrationTarget(uint8_t calibrationTargetIndex) {
    drawTouchCalibrationTargetScreen(
        calibrationTargetIndex,
        STARTUP_TOUCH_TARGET_INSTRUCTION,
        TFT_WHITE,
        STARTUP_TOUCH_TARGET_WAITING_COLOR,
        STARTUP_TOUCH_TARGET_CENTER_COLOR,
        false);
}

void drawTouchCalibrationTargetFeedback(
    uint8_t calibrationTargetIndex,
    const char* feedbackMessage,
    uint16_t feedbackColor,
    bool calibrationPointWasCaptured) {
    drawTouchCalibrationTargetScreen(
        calibrationTargetIndex,
        feedbackMessage,
        feedbackColor,
        feedbackColor,
        feedbackColor,
        calibrationPointWasCaptured);
}

void drawTouchCalibrationTargetRetryPrompt(
    uint8_t calibrationTargetIndex,
    bool touchWasDetected) {
    if (calibrationTargetIndex >= STARTUP_TOUCH_CALIBRATION_TARGET_COUNT) {
        return; // Un indice fuori intervallo non identifica un punto da ripresentare.
    }

    char calibrationTargetRetryTitle[STARTUP_TOUCH_STEP_TITLE_BUFFER_CAPACITY]; // Titolo con il numero del punto da ripetere.
    snprintf(
        calibrationTargetRetryTitle,
        sizeof(calibrationTargetRetryTitle),
        STARTUP_TOUCH_TARGET_RETRY_TITLE_FORMAT,
        static_cast<unsigned int>(calibrationTargetIndex + 1));

    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(STARTUP_TOUCH_TARGET_FAILURE_COLOR, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        calibrationTargetRetryTitle,
        temperatureDisplay.width() / 2,
        58,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        touchWasDetected
            ? STARTUP_TOUCH_TARGET_INVALID_DETAIL
            : STARTUP_TOUCH_TARGET_TIMEOUT_DETAIL,
        temperatureDisplay.width() / 2,
        91,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_TARGET_RETRY_INSTRUCTION,
        temperatureDisplay.width() / 2,
        116,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_BOOT_INSTRUCTION,
        temperatureDisplay.width() / 2,
        141,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    drawFirmwareVersionLabel(STARTUP_PRESENTATION_VERSION_Y_PIXELS);
}

void drawTouchCalibrationSuccess() {
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextColor(TFT_GREEN, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_SUCCESS_TITLE,
        temperatureDisplay.width() / 2,
        76,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_SUCCESS_DETAIL,
        temperatureDisplay.width() / 2,
        112,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    drawFirmwareVersionLabel(STARTUP_PRESENTATION_VERSION_Y_PIXELS);
}

void drawTouchCalibrationRecoveryPrompt() {
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextColor(TFT_YELLOW, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_RETRY_TITLE,
        temperatureDisplay.width() / 2,
        58,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_RETRY_DETAIL,
        temperatureDisplay.width() / 2,
        92,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_RETRY_INSTRUCTION,
        temperatureDisplay.width() / 2,
        117,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_CALIBRATION_BOOT_INSTRUCTION,
        temperatureDisplay.width() / 2,
        142,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    drawFirmwareVersionLabel(STARTUP_PRESENTATION_VERSION_Y_PIXELS);
}

void drawTouchCalibrationBypassed() {
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextColor(TFT_YELLOW, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_BYPASSED_TITLE,
        temperatureDisplay.width() / 2,
        76,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawCentreString(
        STARTUP_TOUCH_BYPASSED_DETAIL,
        temperatureDisplay.width() / 2,
        112,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    drawFirmwareVersionLabel(STARTUP_PRESENTATION_VERSION_Y_PIXELS);
}

static void drawTechnicalMenuInformationRow(
    const char* informationLabel,
    const char* informationValue,
    int16_t rowY,
    uint16_t informationValueColor) {
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TECHNICAL_MENU_LABEL_COLOR, TFT_BLACK);
    temperatureDisplay.drawString(
        informationLabel,
        TECHNICAL_MENU_LABEL_X_PIXELS,
        rowY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(informationValueColor, TFT_BLACK);
    temperatureDisplay.drawRightString(
        informationValue,
        TECHNICAL_MENU_VALUE_RIGHT_X_PIXELS,
        rowY,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.drawFastHLine(
        TECHNICAL_MENU_LABEL_X_PIXELS,
        rowY + TECHNICAL_MENU_ROW_DIVIDER_Y_OFFSET_PIXELS,
        TECHNICAL_MENU_VALUE_RIGHT_X_PIXELS -
            TECHNICAL_MENU_LABEL_X_PIXELS + 1,
        TECHNICAL_MENU_DIVIDER_COLOR);
}

void drawTechnicalMenuScreen() {
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TECHNICAL_MENU_TITLE_COLOR, TFT_BLACK);
    temperatureDisplay.drawString(
        TECHNICAL_MENU_TITLE,
        TECHNICAL_MENU_TITLE_X_PIXELS,
        TECHNICAL_MENU_TITLE_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    drawFirmwareVersionLabel(TECHNICAL_MENU_VERSION_Y_PIXELS);
    temperatureDisplay.drawFastHLine(
        TECHNICAL_MENU_TITLE_X_PIXELS,
        TECHNICAL_MENU_HEADER_RULE_Y_PIXELS,
        temperatureDisplay.width() - 2 * TECHNICAL_MENU_TITLE_X_PIXELS,
        TECHNICAL_MENU_DIVIDER_COLOR);

    char activeProbeCountText[16]; // Testo breve per le sonde attive rispetto al limite configurato.
    snprintf(
        activeProbeCountText,
        sizeof(activeProbeCountText),
        TECHNICAL_MENU_PROBE_COUNT_FORMAT,
        static_cast<unsigned int>(activeTemperatureProbeCount),
        static_cast<unsigned int>(MAXIMUM_PROBE_COUNT));
    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_PROBE_COUNT_LABEL,
        activeProbeCountText,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS,
        TECHNICAL_MENU_VALUE_COLOR);

    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_TOUCH_LABEL,
        resistiveTouchCalibrationIsValid
            ? TECHNICAL_MENU_TOUCH_READY_VALUE
            : TECHNICAL_MENU_TOUCH_UNAVAILABLE_VALUE,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS + TECHNICAL_MENU_ROW_SPACING_PIXELS,
        resistiveTouchCalibrationIsValid
            ? TECHNICAL_MENU_SUCCESS_COLOR
            : TECHNICAL_MENU_WARNING_COLOR);

    char externalSensorBusStatusText[STARTUP_SUMMARY_SENSOR_BUS_TEXT_BUFFER_CAPACITY];
    formatExternalSensorBusStatus(
        startupExternalSensorBusCheckResult,
        externalSensorBusStatusText,
        sizeof(externalSensorBusStatusText));
    const uint16_t externalSensorBusStatusColor =
        startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::OneWire ||
                startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::I2c ||
                (startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::Dht &&
                 startupExternalSensorBusCheckResult.dhtModel !=
                     DhtSensorModel::NotSelected)
            ? TECHNICAL_MENU_SUCCESS_COLOR
            : TECHNICAL_MENU_WARNING_COLOR;
    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_SENSOR_BUS_LABEL,
        externalSensorBusStatusText,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS +
            2 * TECHNICAL_MENU_ROW_SPACING_PIXELS,
        externalSensorBusStatusColor);

    char externalI2cAddressListText[TECHNICAL_MENU_I2C_ADDRESS_TEXT_BUFFER_CAPACITY];
    formatExternalI2cAddressList(
        startupExternalSensorBusCheckResult,
        externalI2cAddressListText,
        sizeof(externalI2cAddressListText));
    const uint16_t externalI2cAddressListColor =
        startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::I2c &&
                startupExternalSensorBusCheckResult.i2cDeviceCount > 0
            ? TECHNICAL_MENU_SUCCESS_COLOR
            : TECHNICAL_MENU_WARNING_COLOR;
    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_I2C_ADDRESSES_LABEL,
        externalI2cAddressListText,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS +
            3 * TECHNICAL_MENU_ROW_SPACING_PIXELS,
        externalI2cAddressListColor);

    const bool microSdFileSystemIsReady =
        startupMicroSdCardCheckResult.status == MicroSdCardCheckStatus::Ready;
    const char* microSdStatusText = TECHNICAL_MENU_MICRO_SD_ABSENT_VALUE;
    uint16_t microSdStatusTextColor = TECHNICAL_MENU_WARNING_COLOR;
    if (microSdFileSystemIsReady) {
        microSdStatusText = TECHNICAL_MENU_MICRO_SD_READY_VALUE;
        microSdStatusTextColor = TECHNICAL_MENU_SUCCESS_COLOR;
    } else if (startupMicroSdCardCheckResult.status == MicroSdCardCheckStatus::FileSystemError) {
        microSdStatusText = TECHNICAL_MENU_MICRO_SD_ERROR_VALUE;
    }

    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_MICRO_SD_LABEL,
        microSdStatusText,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS + 4 * TECHNICAL_MENU_ROW_SPACING_PIXELS,
        microSdStatusTextColor);

    char microSdCapacityAndFreeSpaceText[STARTUP_SUMMARY_STORAGE_TEXT_BUFFER_CAPACITY];
    uint16_t microSdCapacityAndFreeSpaceColor = TECHNICAL_MENU_WARNING_COLOR;
    if (microSdFileSystemIsReady) {
        formatMicroSdCapacityAndFreeSpace(
            startupMicroSdCardCheckResult,
            false,
            microSdCapacityAndFreeSpaceText,
            sizeof(microSdCapacityAndFreeSpaceText));
        microSdCapacityAndFreeSpaceColor = TECHNICAL_MENU_VALUE_COLOR;
    } else {
        snprintf(
            microSdCapacityAndFreeSpaceText,
            sizeof(microSdCapacityAndFreeSpaceText),
            "%s",
            TECHNICAL_MENU_SPACE_UNAVAILABLE_VALUE);
    }

    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_CAPACITY_LABEL,
        microSdCapacityAndFreeSpaceText,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS + 5 * TECHNICAL_MENU_ROW_SPACING_PIXELS,
        microSdCapacityAndFreeSpaceColor);

    const char* microSdDataDirectoryStatusText =
        TECHNICAL_MENU_DIRECTORY_NOT_CHECKED_VALUE;
    uint16_t microSdDataDirectoryStatusColor = TECHNICAL_MENU_WARNING_COLOR;
    if (microSdFileSystemIsReady) {
        switch (startupMicroSdCardCheckResult.dataDirectoryStatus) {
            case MicroSdDataDirectoryStatus::AlreadyPresent:
                microSdDataDirectoryStatusText = TECHNICAL_MENU_DIRECTORY_PRESENT_VALUE;
                microSdDataDirectoryStatusColor = TECHNICAL_MENU_SUCCESS_COLOR;
                break;
            case MicroSdDataDirectoryStatus::Created:
                microSdDataDirectoryStatusText = TECHNICAL_MENU_DIRECTORY_CREATED_VALUE;
                microSdDataDirectoryStatusColor = TECHNICAL_MENU_SUCCESS_COLOR;
                break;
            case MicroSdDataDirectoryStatus::CreationError:
                microSdDataDirectoryStatusText = TECHNICAL_MENU_DIRECTORY_ERROR_VALUE;
                break;
            case MicroSdDataDirectoryStatus::NotChecked:
                break;
        }
    }

    drawTechnicalMenuInformationRow(
        TECHNICAL_MENU_DIRECTORY_LABEL,
        microSdDataDirectoryStatusText,
        TECHNICAL_MENU_FIRST_ROW_Y_PIXELS + 6 * TECHNICAL_MENU_ROW_SPACING_PIXELS,
        microSdDataDirectoryStatusColor);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TECHNICAL_MENU_LABEL_COLOR, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        TECHNICAL_MENU_CLOSE_HINT,
        temperatureDisplay.width() / 2,
        TECHNICAL_MENU_CLOSE_HINT_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

void drawControlBar() {
    const bool touchControlsAreAvailable = resistiveTouchCalibrationIsValid;
    const char* viewButtonLabel = graphViewIsActive
        ? CONTROL_BAR_SWITCH_TO_VALUES_LABEL
        : CONTROL_BAR_SWITCH_TO_GRAPH_LABEL;
    const uint16_t viewButtonBackgroundColor = touchControlsAreAvailable
        ? CONTROL_BAR_BUTTON_BACKGROUND_COLOR
        : CONTROL_BAR_DISABLED_BUTTON_COLOR;
    const uint16_t viewButtonBorderColor = touchControlsAreAvailable
        ? CONTROL_BAR_BUTTON_BORDER_COLOR
        : CONTROL_BAR_DISABLED_BUTTON_COLOR;
    const uint16_t viewButtonTextColor = touchControlsAreAvailable
        ? CONTROL_BAR_BUTTON_TEXT_COLOR
        : CONTROL_BAR_DISABLED_TEXT_COLOR;
    const uint16_t settingsButtonBackgroundColor = technicalMenuIsActive
        ? CONTROL_BAR_SETTINGS_ACTIVE_BACKGROUND_COLOR
        : (touchControlsAreAvailable
            ? CONTROL_BAR_BUTTON_BACKGROUND_COLOR
            : CONTROL_BAR_DISABLED_BUTTON_COLOR);
    const uint16_t settingsButtonBorderColor = touchControlsAreAvailable
        ? CONTROL_BAR_BUTTON_BORDER_COLOR
        : CONTROL_BAR_DISABLED_BUTTON_COLOR;
    const uint16_t settingsIconColor = technicalMenuIsActive
        ? CONTROL_BAR_SETTINGS_ACTIVE_ICON_COLOR
        : (touchControlsAreAvailable
            ? CONTROL_BAR_SETTINGS_ICON_COLOR
            : CONTROL_BAR_DISABLED_TEXT_COLOR);

    temperatureDisplay.fillRect(
        CONTROL_BAR_X_PIXELS,
        CONTROL_BAR_TOP_Y_PIXELS,
        CONTROL_BAR_WIDTH_PIXELS,
        CONTROL_BAR_HEIGHT_PIXELS,
        CONTROL_BAR_BACKGROUND_COLOR);
    temperatureDisplay.drawFastHLine(
        CONTROL_BAR_X_PIXELS,
        CONTROL_BAR_TOP_Y_PIXELS,
        CONTROL_BAR_WIDTH_PIXELS,
        CONTROL_BAR_TOP_BORDER_COLOR);

    temperatureDisplay.fillRoundRect(
        CONTROL_BAR_VIEW_BUTTON_X_PIXELS,
        CONTROL_BAR_BUTTON_Y_PIXELS,
        CONTROL_BAR_VIEW_BUTTON_WIDTH_PIXELS,
        CONTROL_BAR_BUTTON_HEIGHT_PIXELS,
        CONTROL_BAR_BUTTON_CORNER_RADIUS_PIXELS,
        viewButtonBackgroundColor);
    temperatureDisplay.drawRoundRect(
        CONTROL_BAR_VIEW_BUTTON_X_PIXELS,
        CONTROL_BAR_BUTTON_Y_PIXELS,
        CONTROL_BAR_VIEW_BUTTON_WIDTH_PIXELS,
        CONTROL_BAR_BUTTON_HEIGHT_PIXELS,
        CONTROL_BAR_BUTTON_CORNER_RADIUS_PIXELS,
        viewButtonBorderColor);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(viewButtonTextColor, viewButtonBackgroundColor);
    temperatureDisplay.drawCentreString(
        viewButtonLabel,
        CONTROL_BAR_VIEW_BUTTON_CENTER_X_PIXELS,
        CONTROL_BAR_BUTTON_Y_PIXELS + CONTROL_BAR_BUTTON_TEXT_Y_OFFSET_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);

    if (!touchControlsAreAvailable) {
        temperatureDisplay.setTextColor(
            CONTROL_BAR_BOOT_HELP_COLOR,
            CONTROL_BAR_BACKGROUND_COLOR);
        temperatureDisplay.drawCentreString(
            CONTROL_BAR_BOOT_HELP_LABEL,
            CONTROL_BAR_BOOT_HELP_CENTER_X_PIXELS,
            CONTROL_BAR_BUTTON_Y_PIXELS + CONTROL_BAR_BUTTON_TEXT_Y_OFFSET_PIXELS,
            TEMPERATURE_DISPLAY_GRAPH_FONT);
    }

    temperatureDisplay.fillRoundRect(
        CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS,
        CONTROL_BAR_BUTTON_Y_PIXELS,
        CONTROL_BAR_SETTINGS_BUTTON_WIDTH_PIXELS,
        CONTROL_BAR_BUTTON_HEIGHT_PIXELS,
        CONTROL_BAR_BUTTON_CORNER_RADIUS_PIXELS,
        settingsButtonBackgroundColor);
    temperatureDisplay.drawRoundRect(
        CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS,
        CONTROL_BAR_BUTTON_Y_PIXELS,
        CONTROL_BAR_SETTINGS_BUTTON_WIDTH_PIXELS,
        CONTROL_BAR_BUTTON_HEIGHT_PIXELS,
        CONTROL_BAR_BUTTON_CORNER_RADIUS_PIXELS,
        settingsButtonBorderColor);

    const int16_t gearCenterY =
        CONTROL_BAR_BUTTON_Y_PIXELS + CONTROL_BAR_BUTTON_HEIGHT_PIXELS / 2;
    const int16_t gearRadius = CONTROL_BAR_GEAR_ICON_RADIUS_PIXELS;
    const int16_t gearToothLength = CONTROL_BAR_GEAR_ICON_TOOTH_LENGTH_PIXELS;
    const int16_t gearToothOffset = CONTROL_BAR_GEAR_ICON_TOOTH_OFFSET_PIXELS;
    const int16_t gearCenterX = CONTROL_BAR_SETTINGS_BUTTON_CENTER_X_PIXELS;

    temperatureDisplay.drawCircle(
        gearCenterX,
        gearCenterY,
        gearRadius,
        settingsIconColor);
    temperatureDisplay.drawCircle(
        gearCenterX,
        gearCenterY,
        CONTROL_BAR_GEAR_ICON_HOLE_RADIUS_PIXELS,
        settingsIconColor);
    temperatureDisplay.drawFastVLine(
        gearCenterX,
        gearCenterY - CONTROL_BAR_GEAR_ICON_OUTER_RADIUS_PIXELS,
        gearToothLength,
        settingsIconColor);
    temperatureDisplay.drawFastVLine(
        gearCenterX,
        gearCenterY + gearRadius + gearToothOffset,
        gearToothLength,
        settingsIconColor);
    temperatureDisplay.drawFastHLine(
        gearCenterX - CONTROL_BAR_GEAR_ICON_OUTER_RADIUS_PIXELS,
        gearCenterY,
        gearToothLength,
        settingsIconColor);
    temperatureDisplay.drawFastHLine(
        gearCenterX + gearRadius + gearToothOffset,
        gearCenterY,
        gearToothLength,
        settingsIconColor);
    temperatureDisplay.drawLine(
        gearCenterX - gearRadius - gearToothOffset,
        gearCenterY - gearRadius - gearToothOffset,
        gearCenterX - gearRadius,
        gearCenterY - gearRadius,
        settingsIconColor);
    temperatureDisplay.drawLine(
        gearCenterX + gearRadius + gearToothOffset,
        gearCenterY - gearRadius - gearToothOffset,
        gearCenterX + gearRadius,
        gearCenterY - gearRadius,
        settingsIconColor);
    temperatureDisplay.drawLine(
        gearCenterX - gearRadius - gearToothOffset,
        gearCenterY + gearRadius + gearToothOffset,
        gearCenterX - gearRadius,
        gearCenterY + gearRadius,
        settingsIconColor);
    temperatureDisplay.drawLine(
        gearCenterX + gearRadius + gearToothOffset,
        gearCenterY + gearRadius + gearToothOffset,
        gearCenterX + gearRadius,
        gearCenterY + gearRadius,
        settingsIconColor);
}

void drawDhtSensorModelSelectionScreen() {
    const int16_t screenCenterX = temperatureDisplay.width() / 2;
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_TITLE,
        screenCenterX,
        DHT_MODEL_SELECTION_TITLE_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_INSTRUCTION,
        screenCenterX,
        DHT_MODEL_SELECTION_INSTRUCTION_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);

    temperatureDisplay.fillRoundRect(
        DHT_MODEL_SELECTION_DHT11_BUTTON_X_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_Y_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_HEIGHT_PIXELS,
        7,
        DHT_MODEL_SELECTION_DHT11_COLOR);
    temperatureDisplay.drawRoundRect(
        DHT_MODEL_SELECTION_DHT11_BUTTON_X_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_Y_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_HEIGHT_PIXELS,
        7,
        DHT_MODEL_SELECTION_BUTTON_BORDER_COLOR);
    temperatureDisplay.fillRoundRect(
        DHT_MODEL_SELECTION_DHT22_BUTTON_X_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_Y_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_HEIGHT_PIXELS,
        7,
        DHT_MODEL_SELECTION_DHT22_COLOR);
    temperatureDisplay.drawRoundRect(
        DHT_MODEL_SELECTION_DHT22_BUTTON_X_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_Y_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS,
        DHT_MODEL_SELECTION_BUTTON_HEIGHT_PIXELS,
        7,
        DHT_MODEL_SELECTION_BUTTON_BORDER_COLOR);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_WHITE);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_DHT11_LABEL,
        DHT_MODEL_SELECTION_DHT11_BUTTON_X_PIXELS +
            DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS / 2,
        DHT_MODEL_SELECTION_BUTTON_Y_PIXELS + 14,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_DHT22_LABEL,
        DHT_MODEL_SELECTION_DHT22_BUTTON_X_PIXELS +
            DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS / 2,
        DHT_MODEL_SELECTION_BUTTON_Y_PIXELS + 14,
        TEMPERATURE_DISPLAY_TITLE_FONT);

    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_ORANGE, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_NOTICE,
        screenCenterX,
        DHT_MODEL_SELECTION_NOTICE_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_BOOT_HINT,
        screenCenterX,
        DHT_MODEL_SELECTION_BOOT_HINT_Y_PIXELS,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

void drawDhtSensorModelSelectionResult(const char* selectionResultMessage) {
    const int16_t screenCenterX = temperatureDisplay.width() / 2;
    temperatureDisplay.fillScreen(TFT_BLACK);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        DHT_MODEL_SELECTION_TITLE,
        screenCenterX,
        DHT_MODEL_SELECTION_TITLE_Y_PIXELS,
        TEMPERATURE_DISPLAY_TITLE_FONT);
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
    temperatureDisplay.setTextColor(TFT_ORANGE, TFT_BLACK);
    temperatureDisplay.drawCentreString(
        selectionResultMessage,
        screenCenterX,
        DHT_MODEL_SELECTION_INSTRUCTION_Y_PIXELS + 24,
        TEMPERATURE_DISPLAY_GRAPH_FONT);
}

void drawCurrentTemperatureScreen() {
    if (technicalMenuIsActive) {
        drawTechnicalMenuScreen();
    } else {
        temperatureDisplay.fillScreen(TFT_BLACK); // Pulisce la schermata prima di disegnare la nuova vista.
        temperatureDisplay.setTextColor(TFT_WHITE, TFT_BLACK);
        temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);
        temperatureDisplay.drawString(
            graphViewIsActive ? TEMPERATURE_SCREEN_GRAPH_TITLE : TEMPERATURE_SCREEN_NUMERIC_TITLE,
            TEMPERATURE_SCREEN_TITLE_X_PIXELS,
            TEMPERATURE_SCREEN_TITLE_Y_PIXELS);

        if (startupExternalSensorBusCheckResult.mode ==
                ExternalSensorBusMode::Dht &&
            startupExternalSensorBusCheckResult.dhtModel ==
                DhtSensorModel::NotSelected) {
            temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
            temperatureDisplay.setTextColor(TFT_ORANGE, TFT_BLACK);
            temperatureDisplay.drawCentreString(
                TEMPERATURE_SCREEN_DHT_MODEL_REQUIRED_MESSAGE,
                TEMPERATURE_DISPLAY_WIDTH_PIXELS / 2,
                TEMPERATURE_SCREEN_NO_PROBE_TITLE_Y_PIXELS,
                TEMPERATURE_DISPLAY_GRAPH_FONT);
            temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            temperatureDisplay.drawCentreString(
                TEMPERATURE_SCREEN_DHT_MEASUREMENTS_DISABLED_MESSAGE,
                TEMPERATURE_DISPLAY_WIDTH_PIXELS / 2,
                TEMPERATURE_SCREEN_NO_PROBE_HELP_Y_PIXELS,
                TEMPERATURE_DISPLAY_GRAPH_FONT);
        } else if (
            activeTemperatureProbeCount == 0 &&
            (startupExternalSensorBusCheckResult.mode ==
                 ExternalSensorBusMode::OneWire ||
             (startupExternalSensorBusCheckResult.mode ==
                  ExternalSensorBusMode::I2c &&
              startupExternalSensorBusCheckResult.i2cDeviceCount > 0))) {
            temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_GRAPH_FONT);
            temperatureDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
            temperatureDisplay.drawCentreString(
                TEMPERATURE_SCREEN_SENSOR_DETECTED_MESSAGE,
                TEMPERATURE_DISPLAY_WIDTH_PIXELS / 2,
                TEMPERATURE_SCREEN_NO_PROBE_TITLE_Y_PIXELS,
                TEMPERATURE_DISPLAY_GRAPH_FONT);
            temperatureDisplay.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            temperatureDisplay.drawCentreString(
                TEMPERATURE_SCREEN_SENSOR_TYPE_NOT_HANDLED_MESSAGE,
                TEMPERATURE_DISPLAY_WIDTH_PIXELS / 2,
                TEMPERATURE_SCREEN_NO_PROBE_HELP_Y_PIXELS,
                TEMPERATURE_DISPLAY_GRAPH_FONT);
        } else if (activeTemperatureProbeCount == 0) {
            // Mostra uno stato generico quando non sono disponibili misure.
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
    }

    drawControlBar();
    temperatureDisplayNeedsRedraw = false; // La schermata e aggiornata; un nuovo evento potra richiedere un altro disegno.
}

void drawNumericTemperatureValues() {
    temperatureDisplay.setTextFont(TEMPERATURE_DISPLAY_TITLE_FONT);

    if (startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::Dht) {
        char dhtTemperatureText[NUMERIC_TEMPERATURE_TEXT_BUFFER_CAPACITY];
        char dhtHumidityText[NUMERIC_TEMPERATURE_TEXT_BUFFER_CAPACITY];
        const char* selectedDhtModelName = getDhtSensorShortModelName(
            startupExternalSensorBusCheckResult.dhtModel);

        if (isfinite(temperatureProbes[0].latestTemperatureCelsius)) {
            snprintf(
                dhtTemperatureText,
                sizeof(dhtTemperatureText),
                NUMERIC_VIEW_DHT_VALID_TEMPERATURE_FORMAT,
                selectedDhtModelName,
                temperatureProbes[0].latestTemperatureCelsius);
        } else {
            snprintf(
                dhtTemperatureText,
                sizeof(dhtTemperatureText),
                NUMERIC_VIEW_DHT_UNAVAILABLE_TEMPERATURE_FORMAT,
                selectedDhtModelName);
        }

        if (isfinite(latestDhtRelativeHumidityPercent)) {
            snprintf(
                dhtHumidityText,
                sizeof(dhtHumidityText),
                NUMERIC_VIEW_DHT_VALID_HUMIDITY_FORMAT,
                latestDhtRelativeHumidityPercent);
        } else {
            snprintf(
                dhtHumidityText,
                sizeof(dhtHumidityText),
                "%s",
                NUMERIC_VIEW_DHT_UNAVAILABLE_HUMIDITY_FORMAT);
        }

        temperatureDisplay.setTextColor(PROBE_DISPLAY_COLORS[0], TFT_BLACK);
        temperatureDisplay.drawString(
            dhtTemperatureText,
            NUMERIC_VIEW_FIRST_ROW_X_PIXELS,
            NUMERIC_VIEW_DHT_TEMPERATURE_Y_PIXELS);
        temperatureDisplay.setTextColor(TFT_CYAN, TFT_BLACK);
        temperatureDisplay.drawString(
            dhtHumidityText,
            NUMERIC_VIEW_FIRST_ROW_X_PIXELS,
            NUMERIC_VIEW_DHT_HUMIDITY_Y_PIXELS);
        return;
    }

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
    const uint32_t temperatureSampleIntervalMilliseconds =
        startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::Dht
            ? DHT_SENSOR_MINIMUM_READING_INTERVAL_MILLISECONDS
            : TEMPERATURE_SAMPLE_INTERVAL_MILLISECONDS;
    const uint32_t oldestDisplayedSampleAgeSeconds =
        static_cast<uint32_t>(TEMPERATURE_HISTORY_CAPACITY - 1) *
        temperatureSampleIntervalMilliseconds / 1000UL;
    snprintf(oldestSampleTimeLabel,
             sizeof(oldestSampleTimeLabel),
             TEMPERATURE_GRAPH_OLDEST_SAMPLE_TIME_FORMAT,
             static_cast<unsigned int>(oldestDisplayedSampleAgeSeconds));
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

        if (startupExternalSensorBusCheckResult.mode == ExternalSensorBusMode::Dht) {
            const char* selectedDhtModelName = getDhtSensorShortModelName(
                startupExternalSensorBusCheckResult.dhtModel);
            if (isfinite(temperatureProbes[probeIndex].latestTemperatureCelsius)) {
                snprintf(
                    probeLegendText,
                    sizeof(probeLegendText),
                    TEMPERATURE_GRAPH_DHT_VALID_LABEL_FORMAT,
                    selectedDhtModelName,
                    temperatureProbes[probeIndex].latestTemperatureCelsius);
            } else {
                snprintf(
                    probeLegendText,
                    sizeof(probeLegendText),
                    TEMPERATURE_GRAPH_DHT_UNAVAILABLE_LABEL_FORMAT,
                    selectedDhtModelName);
            }
        } else if (isfinite(temperatureProbes[probeIndex].latestTemperatureCelsius)) {
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
