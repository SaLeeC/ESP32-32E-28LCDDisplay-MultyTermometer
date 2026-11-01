/*
 * Modulo della scheda microSD integrata nella ESP32-2432S028R.
 *
 * Il modulo usa il bus SPI dedicato allo slot e monta il filesystem senza
 * formattazione automatica. Se la scheda e utilizzabile, controlla la cartella
 * /SondAris_Data e crea solo quella se manca; non crea file di dati.
 */

#include <SD.h>
#include <SPI.h>
#include <ff.h>
#include <diskio.h>
#include <sd_diskio.h>

namespace {

/*
 * Inizializza il solo protocollo della scheda senza tentare di montare il
 * filesystem. Serve a distinguere una scheda irraggiungibile da un errore FS.
 */
bool microSdCardRespondsToInitialization() {
    const uint8_t diagnosticDriveNumber = sdcard_init(
        MICRO_SD_CARD_CHIP_SELECT_PIN,
        &SPI,
        MICRO_SD_CARD_SPI_FREQUENCY_HERTZ);
    if (diagnosticDriveNumber == 0xFF) {
        return false;
    }

    const DSTATUS cardInitializationStatus = disk_initialize(diagnosticDriveNumber);
    const uint8_t driverCleanupStatus = sdcard_uninit(diagnosticDriveNumber);
    if (driverCleanupStatus != 0) {
        Serial.printf(
            "microSD: errore durante il rilascio del controllo diagnostico (%u).\n",
            static_cast<unsigned int>(driverCleanupStatus));
    }

    return (cardInitializationStatus & (STA_NOINIT | STA_NODISK)) == 0;
}

/*
 * Verifica che la cartella dati esista e sia una directory; se manca la crea.
 * Restituisce: stato della cartella, distinguendo un percorso occupato da un errore.
 * Effetti: crea solo /SondAris_Data quando necessario e ne verifica il risultato.
 */
MicroSdDataDirectoryStatus ensureMicroSdDataDirectoryExists() {
    if (SD.exists(MICRO_SD_DATA_DIRECTORY_PATH)) {
        File existingDataPath = SD.open(MICRO_SD_DATA_DIRECTORY_PATH);
        const bool existingPathIsDirectory =
            existingDataPath && existingDataPath.isDirectory();
        if (existingDataPath) {
            existingDataPath.close();
        }

        if (existingPathIsDirectory) {
            Serial.println("microSD: cartella /SondAris_Data gia presente.");
            return MicroSdDataDirectoryStatus::AlreadyPresent;
        }

        Serial.println(
            "microSD: il nome /SondAris_Data e gia usato da un elemento non-cartella; "
            "non viene modificato.");
        return MicroSdDataDirectoryStatus::CreationError;
    }

    if (!SD.mkdir(MICRO_SD_DATA_DIRECTORY_PATH)) {
        Serial.println("microSD: impossibile creare la cartella /SondAris_Data.");
        return MicroSdDataDirectoryStatus::CreationError;
    }

    File createdDataDirectory = SD.open(MICRO_SD_DATA_DIRECTORY_PATH);
    const bool createdPathIsDirectory =
        createdDataDirectory && createdDataDirectory.isDirectory();
    if (createdDataDirectory) {
        createdDataDirectory.close();
    }

    if (!createdPathIsDirectory) {
        Serial.println(
            "microSD: cartella /SondAris_Data creata ma non verificabile.");
        return MicroSdDataDirectoryStatus::CreationError;
    }

    Serial.println("microSD: cartella /SondAris_Data creata.");
    return MicroSdDataDirectoryStatus::Created;
}

}

/*
 * Controlla la scheda e il filesystem, prepara la cartella dati e riferisce l'esito.
 * La sola scrittura possibile e la creazione di /SondAris_Data.
 */
MicroSdCardCheckResult verifyMicroSdCardAndPrepareDataDirectory() {
    Serial.println(
        "microSD: controllo senza formattazione; puo essere creata solo "
        "/SondAris_Data.");

    SPI.begin(
        MICRO_SD_CARD_CLOCK_PIN,
        MICRO_SD_CARD_MISO_PIN,
        MICRO_SD_CARD_MOSI_PIN,
        MICRO_SD_CARD_CHIP_SELECT_PIN);

    // SD.begin non formatta la scheda: format_if_empty resta false per impostazione predefinita.
    const bool microSdFileSystemMounted = SD.begin(
        MICRO_SD_CARD_CHIP_SELECT_PIN,
        SPI,
        MICRO_SD_CARD_SPI_FREQUENCY_HERTZ);

    if (!microSdFileSystemMounted) {
        if (microSdCardRespondsToInitialization()) {
            Serial.println("microSD: scheda presente, ma il filesystem non e montabile.");
            return {
                MicroSdCardCheckStatus::FileSystemError,
                0,
                0,
                MicroSdDataDirectoryStatus::NotChecked
            };
        }

        Serial.println("microSD: scheda non rilevata o non comunicante.");
        return {
            MicroSdCardCheckStatus::NotDetected,
            0,
            0,
            MicroSdDataDirectoryStatus::NotChecked
        };
    }

    const uint64_t totalFileSystemCapacityBytes = SD.totalBytes();
    const uint64_t usedFileSystemSpaceBeforeDirectoryCheckBytes = SD.usedBytes();
    if (totalFileSystemCapacityBytes == 0 ||
        usedFileSystemSpaceBeforeDirectoryCheckBytes > totalFileSystemCapacityBytes) {
        Serial.println("microSD: filesystem montato, ma capacita o spazio libero non leggibili.");
        return {
            MicroSdCardCheckStatus::FileSystemError,
            0,
            0,
            MicroSdDataDirectoryStatus::NotChecked
        };
    }

    const MicroSdDataDirectoryStatus dataDirectoryStatus =
        ensureMicroSdDataDirectoryExists();
    const uint64_t usedFileSystemSpaceBytes = SD.usedBytes();
    if (usedFileSystemSpaceBytes > totalFileSystemCapacityBytes) {
        Serial.println(
            "microSD: errore nella lettura dello spazio dopo il controllo della cartella.");
        return {
            MicroSdCardCheckStatus::FileSystemError,
            0,
            0,
            dataDirectoryStatus
        };
    }

    Serial.println("microSD: filesystem montato; capacita e spazio residuo letti.");
    Serial.println("microSD: nessun file dati creato o modificato.");
    return {
        MicroSdCardCheckStatus::Ready,
        totalFileSystemCapacityBytes,
        totalFileSystemCapacityBytes - usedFileSystemSpaceBytes,
        dataDirectoryStatus
    };
}
