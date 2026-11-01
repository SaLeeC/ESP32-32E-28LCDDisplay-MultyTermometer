#ifndef ESP32_MULTI_PROBE_THERMOMETER_CONSTANT_H
#define ESP32_MULTI_PROBE_THERMOMETER_CONSTANT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// Versione visualizzata all'avvio e riportata nell'intestazione di src/main.cpp.
constexpr uint8_t FIRMWARE_VERSION_FIRST_NUMBER = 1; // Primo gruppo numerico della versione, mostrato prima del punto.
constexpr uint16_t FIRMWARE_VERSION_SECOND_NUMBER = 34; // Secondo gruppo numerico, mostrato con tre cifre: 034.
constexpr char FIRMWARE_VERSION_RELEASE_DATE[] = "01-11-2026"; // Data associata alla versione, nel formato giorno-mese-anno.
constexpr char FIRMWARE_VERSION_DISPLAY_FORMAT[] = "v%u.%03u %s"; // Formato completo, per esempio v1.029 01-11-2026.
constexpr uint8_t FIRMWARE_VERSION_TEXT_BUFFER_CAPACITY = 32; // Spazio in byte per comporre la versione e il terminatore di stringa.

// Configurazione del bus esterno e delle sonde
constexpr uint32_t SERIAL_MONITOR_BAUD_RATE = 115200; // Velocita del monitor seriale, espressa in bit al secondo.
constexpr uint8_t BATTERY_VOLTAGE_ADC_GPIO_PIN = 34; // Ingresso ADC1 collegato al punto centrale del partitore della batteria.
constexpr uint8_t BATTERY_VOLTAGE_SAMPLE_COUNT = 32; // Campioni mediati all'accensione per ridurre il rumore del convertitore ADC.
constexpr uint16_t BATTERY_VOLTAGE_SAMPLE_INTERVAL_MILLISECONDS = 2; // Pausa tra campioni consecutivi, in millisecondi.
constexpr uint16_t BATTERY_MINIMUM_LOAD_STABILIZATION_MILLISECONDS = 100; // Attesa con retroilluminazione e WiFi spenti prima della misura a vuoto.
constexpr uint16_t BATTERY_MAXIMUM_LOAD_STABILIZATION_MILLISECONDS = 150; // Attesa dopo retroilluminazione e WiFi al massimo prima della misura sotto carico.
constexpr uint8_t ONBOARD_RGB_LED_RED_GPIO_PIN = 22; // Canale rosso attivo LOW; il pin viene rilasciato prima di usare CN1.
constexpr uint8_t ONBOARD_RGB_LED_GREEN_GPIO_PIN = 16; // Canale verde attivo LOW del LED RGB ad anodo comune.
constexpr uint8_t ONBOARD_RGB_LED_BLUE_GPIO_PIN = 17; // Canale blu attivo LOW del LED RGB ad anodo comune.
constexpr uint8_t BATTERY_VOLTAGE_DIVIDER_MULTIPLIER = 2; // Rapporto del partitore formato nello schema da due resistenze uguali da 100 kOhm.
constexpr uint16_t BATTERY_NOT_DETECTED_MAXIMUM_MILLIVOLTS = 2500; // Sotto 2,5 V non e presente una tensione compatibile con una LiPo collegata.
constexpr uint16_t BATTERY_PLAUSIBLE_MAXIMUM_MILLIVOLTS = 4400; // Sopra 4,4 V la lettura non e compatibile con la batteria LiPo da 3,7 V prevista.
constexpr uint8_t EXTERNAL_SENSOR_PRIMARY_GPIO_PIN = 27; // Primo GPIO del connettore CN1 nella configurazione standard.
constexpr uint8_t EXTERNAL_SENSOR_SECONDARY_GPIO_PIN = 22; // Secondo GPIO CN1, scambiato col primario nel secondo tentativo.
constexpr uint8_t DS18B20_DATA_PIN = EXTERNAL_SENSOR_PRIMARY_GPIO_PIN; // GPIO dati OneWire iniziale; il test alternativo usa GPIO22.
constexpr uint8_t EXTERNAL_SENSOR_I2C_SDA_PIN = EXTERNAL_SENSOR_PRIMARY_GPIO_PIN; // SDA I2C nella configurazione standard.
constexpr uint8_t EXTERNAL_SENSOR_I2C_SCL_PIN = EXTERNAL_SENSOR_SECONDARY_GPIO_PIN; // SCL I2C nella configurazione standard.
constexpr uint32_t EXTERNAL_SENSOR_I2C_FREQUENCY_HERTZ = 100000; // Frequenza standard I2C iniziale, in hertz.
constexpr uint8_t EXTERNAL_SENSOR_I2C_FIRST_VALID_ADDRESS = 0x08; // Primo indirizzo dispositivo non riservato da provare nello scan.
constexpr uint8_t EXTERNAL_SENSOR_I2C_LAST_VALID_ADDRESS = 0x77; // Ultimo indirizzo dispositivo non riservato da provare nello scan.
constexpr uint8_t MAXIMUM_EXTERNAL_I2C_DEVICE_COUNT =
    EXTERNAL_SENSOR_I2C_LAST_VALID_ADDRESS -
    EXTERNAL_SENSOR_I2C_FIRST_VALID_ADDRESS + 1; // Numero massimo di indirizzi I2C validi memorizzabili nello scan.
constexpr uint8_t MAXIMUM_EXTERNAL_I2C_SENSOR_COUNT = 8; // Numero massimo di dispositivi I2C registrati con i riferimenti al catalogo.

// Metodo che un futuro riconoscimento potra usare dopo aver trovato un indirizzo I2C.
enum class ExternalI2cSensorIdentificationMethod : uint8_t {
    ProtocolResponse, // Il dispositivo non espone un ID univoco e va verificato con il suo protocollo.
    EightBitRegister, // Un registro a 8 bit restituisce un valore identificativo confrontabile.
    SixteenBitRegister, // Un registro a 8 bit restituisce un valore identificativo a 16 bit.
    SerialNumberCommand, // Un comando specifico restituisce un numero seriale protetto da CRC.
    ProductIdentifierCommand // Un comando specifico restituisce un identificatore di prodotto.
};

// Seleziona il driver compilato che sa inizializzare e leggere il modello I2C.
enum class ExternalI2cSensorDriverKind : uint8_t {
    AdafruitMcp9808, // Driver Adafruit per MCP9808.
    AdafruitAhtx0, // Driver Adafruit condiviso da AHT10, AHT20 e DHT20.
    AdafruitSi7021, // Driver Adafruit specifico per Si7021.
    AdafruitHtu21df, // Driver Adafruit specifico per HTU21D/HTU21DF.
    GenericHdc1080, // Driver ClosedCube distribuito come Generic HDC1080.
    AdafruitSht31, // Driver Adafruit per il protocollo Sensirion SHT3x.
    AdafruitSht4x, // Driver Adafruit per il protocollo Sensirion SHT4x.
    SparkFunTmp102, // Driver SparkFun per TMP102.
    SparkFunTmp117, // Driver SparkFun per TMP117.
    Lm75Derived, // Driver comune per i dispositivi derivati da LM75.
    AdafruitAm2320, // Driver Adafruit per AM2320.
    RobTillaartDht12, // Driver RobTillaart per DHT12 in modalita I2C.
    AdafruitShtc3, // Driver Adafruit per SHTC3.
    AdafruitBmp280, // Driver Adafruit per BMP280.
    AdafruitBme280, // Driver Adafruit per BME280.
    BoschBme68x // Driver ufficiale Bosch condiviso da BME680 e BME688.
};

// Descrive un modello I2C noto e le informazioni utili a riconoscerlo e leggerlo.
struct ExternalI2cSensorCatalogEntry {
    const char* familyName; // Nome leggibile del modello o del suo alias commerciale.
    ExternalI2cSensorDriverKind driverKind; // Driver compilato da associare al dispositivo dopo l'identificazione.
    uint8_t firstAddress; // Primo indirizzo I2C a 7 bit assegnabile al modello.
    uint8_t lastAddress; // Ultimo indirizzo I2C a 7 bit assegnabile al modello.
    bool measuresTemperature; // True se il dispositivo fornisce la temperatura.
    bool measuresHumidity; // True se il dispositivo fornisce l'umidita relativa.
    bool measuresPressure; // True se il dispositivo fornisce la pressione atmosferica.
    bool measuresGas; // True se il dispositivo comprende anche una misura della resistenza gas.
    float minimumTemperatureCelsius; // Temperatura minima misurabile dichiarata, in gradi Celsius.
    float maximumTemperatureCelsius; // Temperatura massima misurabile dichiarata, in gradi Celsius.
    float nominalTemperatureAccuracyCelsius; // Errore nominale in valore assoluto, in gradi Celsius; puo valere nell'intervallo centrale del sensore.
    float temperatureResolutionCelsius; // Minima variazione rappresentata dal dato di temperatura, in gradi Celsius.
    ExternalI2cSensorIdentificationMethod identificationMethod; // Tipo di verifica richiesta oltre alla risposta all'indirizzo.
    uint16_t identificationRegisterOrCommand; // Registro o comando da usare; zero se la verifica dipende solo dal protocollo.
    uint16_t expectedIdentificationValue; // Valore atteso prima della maschera; zero se non esiste un valore fisso.
    uint16_t identificationValueMask; // Bit significativi del valore atteso; zero indica una risposta senza valore fisso.
};

/*
 * Catalogo dei sensori di temperatura e umidita previsti.
 * L'indirizzo restringe i candidati ma non identifica sempre il modello:
 * le routine di validazione devono applicare anche il metodo indicato.
 */
constexpr ExternalI2cSensorCatalogEntry EXTERNAL_I2C_SENSOR_CATALOG[] = {
    {"MCP9808", ExternalI2cSensorDriverKind::AdafruitMcp9808, 0x18, 0x1F, true, false, false, false, -40.0f, 125.0f, 0.25f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::SixteenBitRegister, 0x07, 0x0400, 0xFF00},
    {"AHT10", ExternalI2cSensorDriverKind::AdafruitAhtx0, 0x38, 0x38, true, true, false, false, -40.0f, 85.0f, 0.3f, 0.01f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"AHT20", ExternalI2cSensorDriverKind::AdafruitAhtx0, 0x38, 0x38, true, true, false, false, -40.0f, 85.0f, 0.3f, 0.01f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"DHT20", ExternalI2cSensorDriverKind::AdafruitAhtx0, 0x38, 0x38, true, true, false, false, -40.0f, 80.0f, 0.5f, 0.01f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"Si7021", ExternalI2cSensorDriverKind::AdafruitSi7021, 0x40, 0x40, true, true, false, false, -40.0f, 125.0f, 0.4f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0xFA0F, 0x0000, 0x0000},
    {"HTU21D", ExternalI2cSensorDriverKind::AdafruitHtu21df, 0x40, 0x40, true, true, false, false, -40.0f, 125.0f, 0.3f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0xFA0F, 0x0000, 0x0000},
    {"HDC1080", ExternalI2cSensorDriverKind::GenericHdc1080, 0x40, 0x40, true, true, false, false, -40.0f, 125.0f, 0.2f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SixteenBitRegister, 0x00FF, 0x1050, 0xFFFF},
    {"SHT30", ExternalI2cSensorDriverKind::AdafruitSht31, 0x44, 0x45, true, true, false, false, -40.0f, 125.0f, 0.3f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0x3780, 0x0000, 0x0000},
    {"SHT31", ExternalI2cSensorDriverKind::AdafruitSht31, 0x44, 0x45, true, true, false, false, -40.0f, 125.0f, 0.3f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0x3780, 0x0000, 0x0000},
    {"SHT35", ExternalI2cSensorDriverKind::AdafruitSht31, 0x44, 0x45, true, true, false, false, -40.0f, 125.0f, 0.1f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0x3780, 0x0000, 0x0000},
    {"SHT40", ExternalI2cSensorDriverKind::AdafruitSht4x, 0x44, 0x44, true, true, false, false, -40.0f, 125.0f, 0.2f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0x0089, 0x0000, 0x0000},
    {"SHT41", ExternalI2cSensorDriverKind::AdafruitSht4x, 0x44, 0x44, true, true, false, false, -40.0f, 125.0f, 0.2f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0x0089, 0x0000, 0x0000},
    {"SHT45", ExternalI2cSensorDriverKind::AdafruitSht4x, 0x44, 0x44, true, true, false, false, -40.0f, 125.0f, 0.1f, 0.01f,
     ExternalI2cSensorIdentificationMethod::SerialNumberCommand, 0x0089, 0x0000, 0x0000},
    {"TMP102", ExternalI2cSensorDriverKind::SparkFunTmp102, 0x48, 0x4B, true, false, false, false, -40.0f, 125.0f, 0.5f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"TMP117", ExternalI2cSensorDriverKind::SparkFunTmp117, 0x48, 0x4B, true, false, false, false, -55.0f, 150.0f, 0.1f, 0.0078125f,
     ExternalI2cSensorIdentificationMethod::SixteenBitRegister, 0x000F, 0x0117, 0xFFFF},
    {"LM75", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.5f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"LM75A", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.125f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"LM75B", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.125f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"TMP75", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -40.0f, 125.0f, 2.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"TMP75B", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"TMP175", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -40.0f, 125.0f, 1.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"TMP275", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -40.0f, 125.0f, 0.5f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"PCT2075", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 1.0f, 0.125f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"TCN75A", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 1.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"MCP9800", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 1.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"MCP9801", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 1.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"MCP9802", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 1.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"MCP9803", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 1.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"STLM75", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.5f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"STDS75", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 3.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"ADT75", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"DS75", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 2.0f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"DS7505", ExternalI2cSensorDriverKind::Lm75Derived, 0x48, 0x4F, true, false, false, false, -55.0f, 125.0f, 0.5f, 0.0625f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"AM2320", ExternalI2cSensorDriverKind::AdafruitAm2320, 0x5C, 0x5C, true, true, false, false, -40.0f, 80.0f, 0.5f, 0.1f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"DHT12 I2C", ExternalI2cSensorDriverKind::RobTillaartDht12, 0x5C, 0x5C, true, true, false, false, -20.0f, 60.0f, 0.5f, 0.1f,
     ExternalI2cSensorIdentificationMethod::ProtocolResponse, 0x0000, 0x0000, 0x0000},
    {"SHTC3", ExternalI2cSensorDriverKind::AdafruitShtc3, 0x70, 0x70, true, true, false, false, -40.0f, 125.0f, 0.2f, 0.01f,
     ExternalI2cSensorIdentificationMethod::ProductIdentifierCommand, 0xEFC8, 0x0807, 0x083F},
    {"BMP280", ExternalI2cSensorDriverKind::AdafruitBmp280, 0x76, 0x77, true, false, true, false, -40.0f, 85.0f, 1.0f, 0.01f,
     ExternalI2cSensorIdentificationMethod::EightBitRegister, 0x00D0, 0x0058, 0x00FF},
    {"BME280", ExternalI2cSensorDriverKind::AdafruitBme280, 0x76, 0x77, true, true, true, false, -40.0f, 85.0f, 1.0f, 0.01f,
     ExternalI2cSensorIdentificationMethod::EightBitRegister, 0x00D0, 0x0060, 0x00FF},
    {"BME680 / BME688", ExternalI2cSensorDriverKind::BoschBme68x, 0x76, 0x77, true, true, true, true, -40.0f, 85.0f, 1.0f, 0.01f,
     ExternalI2cSensorIdentificationMethod::EightBitRegister, 0x00D0, 0x0061, 0x00FF}
};
constexpr size_t EXTERNAL_I2C_SENSOR_CATALOG_ENTRY_COUNT =
    sizeof(EXTERNAL_I2C_SENSOR_CATALOG) /
    sizeof(EXTERNAL_I2C_SENSOR_CATALOG[0]); // Numero di modelli disponibili nel catalogo per le future ricerche.

constexpr uint8_t MAXIMUM_PROBE_COUNT = 8; // Numero massimo di sonde che il firmware puo gestire.
constexpr uint8_t MAXIMUM_ONEWIRE_SENSOR_COUNT = 8; // Numero massimo di dispositivi OneWire registrati con i riferimenti al catalogo.
constexpr uint8_t DS18B20_FAMILY_CODE = 0x28; // Codice nel ROM che identifica una sonda della famiglia DS18B20.
constexpr uint8_t DS18B20_RESOLUTION_BITS = 12; // Risoluzione usata per la misura; a 12 bit servono fino a 750 ms.
constexpr float DS18B20_MINIMUM_TEMPERATURE_CELSIUS = -55.0f; // Limite inferiore valido dichiarato per una DS18B20, in gradi Celsius.
constexpr float DS18B20_MAXIMUM_TEMPERATURE_CELSIUS = 125.0f; // Limite superiore valido dichiarato per una DS18B20, in gradi Celsius.
constexpr uint32_t DS18B20_CONVERSION_DURATION_MILLISECONDS = 750; // Attesa necessaria prima di leggere una conversione a 12 bit.
constexpr uint32_t TEMPERATURE_SAMPLE_INTERVAL_MILLISECONDS = 1000; // Intervallo tra l'avvio di due misure, in millisecondi.
constexpr uint16_t TEMPERATURE_HISTORY_CAPACITY = 120; // Campioni conservati per sonda: circa 2 minuti OneWire o 4 minuti DHT22.

// Seleziona il driver disponibile per un modello Dallas/Maxim 1-Wire.
enum class OneWireSensorDriverKind : uint8_t {
    DallasTemperature, // Driver condiviso per le famiglie gestite dalla libreria DallasTemperature.
    CustomMax31825, // Adattatore futuro per formato e indirizzamento specifici del MAX31825.
    CustomMax31826, // Adattatore futuro per MAX31826 e la sua EEPROM integrata.
    CustomDs1923 // Driver futuro per temperatura, umidita e memoria missione del DS1923.
};

// Descrive una famiglia di sensori compatibile con il protocollo Dallas/Maxim 1-Wire.
struct OneWireSensorCatalogEntry {
    const char* familyName; // Nome leggibile del dispositivo candidato.
    OneWireSensorDriverKind driverKind; // Driver compilato o adattatore richiesto per consultare il modello.
    uint8_t familyCode; // Primo byte dell'indirizzo ROM a 64 bit, assegnato dal produttore alla famiglia.
    bool measuresTemperature; // True se il dispositivo fornisce la temperatura.
    bool measuresHumidity; // True se il dispositivo fornisce anche l'umidita relativa.
    float minimumTemperatureCelsius; // Temperatura minima misurabile dichiarata, in gradi Celsius.
    float maximumTemperatureCelsius; // Temperatura massima misurabile dichiarata, in gradi Celsius.
    float nominalTemperatureAccuracyCelsius; // Errore nominale in valore assoluto, in gradi Celsius; puo valere nell'intervallo centrale del sensore.
    float temperatureResolutionCelsius; // Migliore risoluzione configurabile o disponibile, in gradi Celsius.
};

/*
 * Catalogo iniziale dei principali sensori Dallas/Maxim 1-Wire.
 * Il codice famiglia puo essere condiviso da modelli diversi: una futura
 * identificazione dovra conservare tutti i candidati con lo stesso codice.
 * DHT11 e DHT22 non compaiono perche usano un protocollo proprietario
 * single-wire, incompatibile con il bus 1-Wire e con la libreria OneWire.
 */
constexpr OneWireSensorCatalogEntry ONE_WIRE_SENSOR_CATALOG[] = {
    {"DS18S20", OneWireSensorDriverKind::DallasTemperature, 0x10, true, false, -55.0f, 125.0f, 0.5f, 0.5f},
    {"DS1822", OneWireSensorDriverKind::DallasTemperature, 0x22, true, false, -55.0f, 125.0f, 2.0f, 0.0625f},
    {"MAX31826", OneWireSensorDriverKind::CustomMax31826, 0x26, true, false, -55.0f, 125.0f, 0.5f, 0.0625f},
    {"DS18B20", OneWireSensorDriverKind::DallasTemperature, 0x28, true, false, -55.0f, 125.0f, 0.5f, 0.0625f},
    {"MAX31820", OneWireSensorDriverKind::DallasTemperature, 0x28, true, false, -55.0f, 125.0f, 0.5f, 0.0625f},
    {"DS1825", OneWireSensorDriverKind::DallasTemperature, 0x3B, true, false, -55.0f, 125.0f, 0.5f, 0.0625f},
    {"MAX31825", OneWireSensorDriverKind::CustomMax31825, 0x3B, true, false, -45.0f, 145.0f, 1.0f, 0.0625f},
    {"DS1923 Hygrochron", OneWireSensorDriverKind::CustomDs1923, 0x41, true, true, -20.0f, 85.0f, 0.5f, 0.0625f},
    {"DS28EA00", OneWireSensorDriverKind::DallasTemperature, 0x42, true, false, -40.0f, 85.0f, 3.0f, 0.0625f}
};
constexpr size_t ONE_WIRE_SENSOR_CATALOG_ENTRY_COUNT =
    sizeof(ONE_WIRE_SENSOR_CATALOG) /
    sizeof(ONE_WIRE_SENSOR_CATALOG[0]); // Numero di dispositivi candidati presenti nel catalogo 1-Wire.

// Identifica il formato dati e le temporizzazioni proprie di ciascun modello DHT.
enum class DhtSensorModel : uint8_t {
    NotSelected, // Il sensore DHT e stato rilevato, ma l'operatore non ha ancora scelto il modello.
    Dht11, // Modello base con dati interi e intervallo di misura ridotto.
    Dht12, // Modello con decimali che puo comunicare anche tramite I2C.
    Dht21, // Modello noto anche come AM2301, con intervallo esteso.
    Dht22 // Modello noto anche come AM2302, con migliore accuratezza dell'umidita.
};

// Seleziona il driver previsto per il formato dati del modello DHT.
enum class DhtSensorDriverKind : uint8_t {
    AdafruitDht, // Driver Adafruit per DHT11, DHT21 e DHT22 single-wire.
    CustomDht12SingleWire, // Adattatore futuro per il formato single-wire DHT12.
    RobTillaartDht12I2c // Driver RobTillaart per la modalita I2C del DHT12.
};

// Descrive un sensore che usa il protocollo proprietario single-wire della famiglia DHT.
struct DhtSensorCatalogEntry {
    DhtSensorModel model; // Valore usato dal driver per scegliere formato dati e limiti di misura.
    DhtSensorDriverKind driverKind; // Driver da usare dopo la selezione o identificazione del modello.
    const char* modelName; // Nome leggibile del sensore e degli eventuali equivalenti commerciali.
    bool supportsProprietarySingleWire; // True se il modello usa il protocollo temporizzato DHT su un filo dati.
    bool supportsI2c; // True se il modello offre anche una modalita I2C separata.
    uint8_t i2cAddress; // Indirizzo I2C a 7 bit, oppure zero quando la modalita I2C non esiste.
    float minimumTemperatureCelsius; // Temperatura minima misurabile dichiarata, in gradi Celsius.
    float maximumTemperatureCelsius; // Temperatura massima misurabile dichiarata, in gradi Celsius.
    float nominalTemperatureAccuracyCelsius; // Errore nominale in valore assoluto, in gradi Celsius.
    float temperatureResolutionCelsius; // Minima variazione rappresentata dal dato di temperatura, in gradi Celsius.
    float minimumRelativeHumidityPercent; // Umidita relativa minima misurabile, in percentuale.
    float maximumRelativeHumidityPercent; // Umidita relativa massima misurabile, in percentuale.
    float nominalHumidityAccuracyPercent; // Errore nominale dell'umidita relativa, in punti percentuali.
    float humidityResolutionPercent; // Minima variazione rappresentata dal dato di umidita, in punti percentuali.
};

/*
 * Catalogo iniziale dei sensori DHT piu diffusi.
 * Questi dispositivi non hanno il ROM univoco del bus Dallas/Maxim 1-Wire:
 * nella modalita single-wire il modello deve essere configurato e ogni sensore
 * richiede una propria linea dati. Solo DHT12 offre anche la modalita I2C.
 * Il rilevamento non distingue i modelli: DHT11 o DHT22 va scelto dall'operatore.
 */
constexpr DhtSensorCatalogEntry DHT_SENSOR_CATALOG[] = {
    {DhtSensorModel::Dht11, DhtSensorDriverKind::AdafruitDht, "DHT11", true, false, 0x00,
     0.0f, 50.0f, 2.0f, 1.0f, 20.0f, 90.0f, 5.0f, 1.0f},
    {DhtSensorModel::Dht12, DhtSensorDriverKind::CustomDht12SingleWire, "DHT12", true, true, 0x5C,
     -20.0f, 60.0f, 0.5f, 0.1f, 20.0f, 95.0f, 5.0f, 0.1f},
    {DhtSensorModel::Dht21, DhtSensorDriverKind::AdafruitDht, "DHT21 / AM2301", true, false, 0x00,
     -40.0f, 80.0f, 0.5f, 0.1f, 0.0f, 100.0f, 3.0f, 0.1f},
    {DhtSensorModel::Dht22, DhtSensorDriverKind::AdafruitDht, "DHT22 / AM2302", true, false, 0x00,
     -40.0f, 80.0f, 0.5f, 0.1f, 0.0f, 100.0f, 2.0f, 0.1f}
};
constexpr size_t DHT_SENSOR_CATALOG_ENTRY_COUNT =
    sizeof(DHT_SENSOR_CATALOG) /
    sizeof(DHT_SENSOR_CATALOG[0]); // Numero di modelli disponibili nel catalogo DHT.

// Parametri di rilevamento, lettura e scelta operatore per i sensori DHT.
constexpr uint32_t DHT_SENSOR_START_SIGNAL_DURATION_MILLISECONDS = 20; // Impulso iniziale compatibile con DHT11 e DHT22, in millisecondi.
constexpr uint32_t DHT_SENSOR_PULSE_TIMEOUT_MICROSECONDS = 150; // Attesa massima per ciascun impulso del protocollo, in microsecondi.
constexpr uint32_t DHT_SENSOR_MINIMUM_VALID_PULSE_MICROSECONDS = 10; // Durata minima plausibile per un impulso dati, in microsecondi.
constexpr uint32_t DHT_SENSOR_MAXIMUM_VALID_PULSE_MICROSECONDS = 120; // Durata massima plausibile per un impulso dati, in microsecondi.
constexpr uint32_t DHT_SENSOR_MINIMUM_RESPONSE_PULSE_MICROSECONDS = 50; // Durata minima dei due impulsi di risposta iniziale, in microsecondi.
constexpr uint32_t DHT_SENSOR_LOGIC_ONE_THRESHOLD_MICROSECONDS = 50; // Un impulso alto piu lungo indica il bit logico 1, in microsecondi.
constexpr uint32_t DHT_SENSOR_MINIMUM_READING_INTERVAL_MILLISECONDS = 2000; // Intervallo prudenziale condiviso da DHT11 e DHT22, in millisecondi.
constexpr uint32_t DHT_MODEL_SELECTION_TIMEOUT_MILLISECONDS = 30000; // Dopo 30 secondi senza scelta le misure restano disattivate.
constexpr uint32_t DHT_MODEL_SELECTION_FEEDBACK_DURATION_MILLISECONDS = 1500; // Durata del messaggio dopo scelta, timeout o esclusione.
constexpr int16_t DHT_MODEL_SELECTION_TITLE_Y_PIXELS = 24; // Coordinata verticale del titolo della scelta modello, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_INSTRUCTION_Y_PIXELS = 61; // Coordinata verticale dell'istruzione per l'operatore, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_BUTTON_Y_PIXELS = 101; // Coordinata verticale dei due pulsanti modello, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_BUTTON_WIDTH_PIXELS = 120; // Larghezza di ciascun pulsante modello, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_BUTTON_HEIGHT_PIXELS = 48; // Altezza di ciascun pulsante modello, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_DHT11_BUTTON_X_PIXELS = 24; // Bordo sinistro del pulsante DHT11, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_DHT22_BUTTON_X_PIXELS = 176; // Bordo sinistro del pulsante DHT22, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_NOTICE_Y_PIXELS = 171; // Coordinata verticale dell'avviso sulle misure, in pixel.
constexpr int16_t DHT_MODEL_SELECTION_BOOT_HINT_Y_PIXELS = 204; // Coordinata verticale dell'istruzione BOOT, in pixel.
constexpr uint16_t DHT_MODEL_SELECTION_BUTTON_BORDER_COLOR = TFT_CYAN; // Bordo comune per distinguere i pulsanti selezionabili.
constexpr uint16_t DHT_MODEL_SELECTION_DHT11_COLOR = TFT_DARKCYAN; // Colore del pulsante DHT11.
constexpr uint16_t DHT_MODEL_SELECTION_DHT22_COLOR = TFT_DARKGREEN; // Colore del pulsante DHT22.
constexpr char DHT_MODEL_SELECTION_TITLE[] = "SENSORE DHT RILEVATO"; // Titolo della schermata che chiede il modello.
constexpr char DHT_MODEL_SELECTION_INSTRUCTION[] = "Seleziona il modello collegato"; // Istruzione per la scelta dell'operatore.
constexpr char DHT_MODEL_SELECTION_DHT11_LABEL[] = "DHT11"; // Etichetta touch per selezionare il modello DHT11.
constexpr char DHT_MODEL_SELECTION_DHT22_LABEL[] = "DHT22"; // Etichetta touch per selezionare il modello DHT22.
constexpr char DHT_MODEL_SELECTION_NOTICE[] = "Senza scelta le misure restano disattivate"; // Spiega l'effetto di una selezione mancante.
constexpr char DHT_MODEL_SELECTION_BOOT_HINT[] = "BOOT: prosegui senza misure"; // Consente di proseguire senza configurare il DHT.
constexpr char DHT_MODEL_SELECTION_NO_TOUCH_MESSAGE[] = "Touch non calibrato: misure disattivate"; // Esito quando la calibrazione touch e stata esclusa o non riuscita.
constexpr char DHT_MODEL_SELECTION_TIMEOUT_MESSAGE[] = "Tempo scaduto: misure disattivate"; // Esito quando il tempo di scelta termina.
constexpr char DHT_MODEL_SELECTION_BOOT_MESSAGE[] = "Scelta annullata: misure disattivate"; // Esito quando BOOT esclude la scelta del modello.
constexpr char DHT_MODEL_SELECTION_COMPLETE_MESSAGE[] = "Modello configurato"; // Conferma visiva dopo la selezione del modello.
constexpr char DHT_MODEL_SELECTION_ERROR_MESSAGE[] = "Modello non configurabile"; // Errore esplicito se il modello scelto non e supportato.
constexpr char EXTERNAL_SENSOR_BUS_DHT_PENDING_FORMAT[] = "DHT: scegliere 11/22 GPIO%u"; // Stato DHT in attesa con il GPIO effettivamente rilevato.
constexpr char EXTERNAL_SENSOR_BUS_DHT_SELECTED_FORMAT[] = "%s GPIO%u"; // Stato compatto del DHT configurato sul bus esterno.
constexpr uint32_t STARTUP_SENSOR_RETRY_SUMMARY_DURATION_MILLISECONDS = 1800; // Tempo per leggere un nuovo esito dopo la pressione di Start.
constexpr uint32_t STARTUP_NO_SENSOR_POLL_INTERVAL_MILLISECONDS = 10; // Intervallo di attesa mentre si aspettano Start o BOOT, in millisecondi.
constexpr int16_t STARTUP_NO_SENSOR_CARD_X_PIXELS = 12; // Bordo sinistro della scheda grafica senza sensori, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_CARD_Y_PIXELS = 12; // Bordo superiore della scheda grafica senza sensori, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_CARD_WIDTH_PIXELS = 296; // Larghezza della scheda grafica senza sensori, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_CARD_HEIGHT_PIXELS = 214; // Altezza della scheda grafica senza sensori, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_ICON_CENTER_X_PIXELS =
    STARTUP_NO_SENSOR_CARD_X_PIXELS + STARTUP_NO_SENSOR_CARD_WIDTH_PIXELS / 2; // Centro orizzontale dell'icona ricerca.
constexpr int16_t STARTUP_NO_SENSOR_ICON_CENTER_Y_PIXELS = 43; // Centro verticale dell'icona ricerca, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_TITLE_Y_PIXELS = 68; // Coordinata verticale del titolo, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_FIRST_INSTRUCTION_Y_PIXELS = 100; // Prima riga di invito al collegamento, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_SECOND_INSTRUCTION_Y_PIXELS = 122; // Seconda riga di invito al collegamento, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_START_BUTTON_X_PIXELS = 92; // Bordo sinistro del pulsante Start, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_START_BUTTON_Y_PIXELS = 153; // Bordo superiore del pulsante Start, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_START_BUTTON_WIDTH_PIXELS = 136; // Larghezza touch del pulsante Start, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_START_BUTTON_HEIGHT_PIXELS = 42; // Altezza touch del pulsante Start, in pixel.
constexpr int16_t STARTUP_NO_SENSOR_BOOT_HINT_Y_PIXELS = 208; // Coordinata verticale dell'alternativa fisica BOOT, in pixel.
constexpr uint16_t STARTUP_NO_SENSOR_CARD_COLOR = TFT_NAVY; // Fondo blu scuro della scheda informativa.
constexpr uint16_t STARTUP_NO_SENSOR_BORDER_COLOR = TFT_DARKCYAN; // Bordo della scheda e dei dettagli decorativi.
constexpr uint16_t STARTUP_NO_SENSOR_ACCENT_COLOR = TFT_CYAN; // Colore principale dei messaggi e dell'icona.
constexpr uint16_t STARTUP_NO_SENSOR_BUTTON_COLOR = TFT_DARKGREEN; // Fondo del pulsante Start.
constexpr uint16_t STARTUP_NO_SENSOR_BUTTON_BORDER_COLOR = TFT_GREEN; // Bordo luminoso del pulsante Start.
constexpr char STARTUP_NO_SENSOR_TITLE[] = "Nessun sensore rilevato."; // Prima parte del messaggio richiesto dall'operatore.
constexpr char STARTUP_NO_SENSOR_FIRST_INSTRUCTION[] = "Collegate dei sensori"; // Seconda parte del messaggio richiesto dall'operatore.
constexpr char STARTUP_NO_SENSOR_SECOND_INSTRUCTION[] = "e premete Start"; // Conclude l'invito a collegare sensori e ripetere la ricerca.
constexpr char STARTUP_NO_SENSOR_START_BUTTON_LABEL[] = "START"; // Etichetta visibile del pulsante che ripete la scansione.
constexpr char STARTUP_NO_SENSOR_BOOT_HINT[] = "Oppure premete BOOT per riprovare"; // Alternativa fisica se il touch non e utilizzabile.
constexpr char STARTUP_SENSOR_SEARCH_PROGRESS_MESSAGE[] = "Sto cercando i sensori..."; // Feedback durante la nuova scansione su entrambi i pin.

// Collegamenti SPI dello slot microSD sulla scheda ESP32-2432S028R.
constexpr uint8_t MICRO_SD_CARD_CHIP_SELECT_PIN = 5; // Seleziona la scheda microSD sul bus SPI.
constexpr uint8_t MICRO_SD_CARD_CLOCK_PIN = 18; // Clock del bus SPI della microSD.
constexpr uint8_t MICRO_SD_CARD_MISO_PIN = 19; // Dati dalla microSD verso l'ESP32.
constexpr uint8_t MICRO_SD_CARD_MOSI_PIN = 23; // Dati dall'ESP32 verso la microSD.
constexpr char MICRO_SD_DATA_DIRECTORY_PATH[] = "/SondAris_Data"; // Cartella dati nella radice della microSD, creata se manca.
constexpr uint32_t MICRO_SD_CARD_SPI_FREQUENCY_HERTZ = 4000000; // Frequenza iniziale SPI della microSD, in hertz.
constexpr uint64_t MICRO_SD_BYTES_PER_KILOBYTE = 1024ULL; // Byte per l'unita KB usata nel riepilogo.
constexpr uint64_t MICRO_SD_BYTES_PER_MEGABYTE = MICRO_SD_BYTES_PER_KILOBYTE * 1024ULL; // Byte per l'unita MB usata nel riepilogo.
constexpr uint64_t MICRO_SD_BYTES_PER_GIGABYTE = MICRO_SD_BYTES_PER_MEGABYTE * 1024ULL; // Byte per l'unita GB usata nel riepilogo.
constexpr uint64_t MICRO_SD_BYTES_PER_TERABYTE = MICRO_SD_BYTES_PER_GIGABYTE * 1024ULL; // Byte per l'unita TB usata nel riepilogo.
constexpr uint64_t MICRO_SD_LOW_FREE_SPACE_LIMIT_BYTES = 100ULL * MICRO_SD_BYTES_PER_MEGABYTE; // Soglia di allerta: 100 MB convertiti con multipli di 1024.

// Configurazione del pulsante BOOT
constexpr uint8_t BOOT_BUTTON_PIN = 0; // GPIO0 del pulsante BOOT, che vale LOW quando viene premuto.
constexpr uint32_t BOOT_BUTTON_DEBOUNCE_DURATION_MILLISECONDS = 40; // Stabilita richiesta per ignorare il rimbalzo elettrico del pulsante.

// Configurazione generale del display
constexpr uint8_t TEMPERATURE_DISPLAY_LANDSCAPE_ROTATION = 1; // Rotazione orizzontale usata dal display 320 x 240.
constexpr int16_t TEMPERATURE_DISPLAY_WIDTH_PIXELS = 320; // Larghezza del display in orientamento orizzontale, in pixel.
constexpr int16_t TEMPERATURE_DISPLAY_HEIGHT_PIXELS = 240; // Altezza del display in orientamento orizzontale, in pixel.
constexpr uint8_t TEMPERATURE_DISPLAY_TITLE_FONT = 2; // Font TFT_eSPI per titolo e valori numerici.
constexpr uint8_t TEMPERATURE_DISPLAY_GRAPH_FONT = 1; // Font TFT_eSPI piu piccolo, usato nel grafico e nelle etichette.
constexpr uint16_t PROBE_DISPLAY_COLORS[MAXIMUM_PROBE_COUNT] = { // Colori stabili per riconoscere ogni sonda nelle due viste.
    TFT_RED, TFT_GREEN, TFT_CYAN, TFT_YELLOW,
    TFT_MAGENTA, TFT_ORANGE, TFT_WHITE, TFT_PINK
};

// Collegamenti del controller touch XPT2046 sulla scheda ESP32-2432S028R.
constexpr uint8_t XPT2046_TOUCH_CHIP_SELECT_PIN = 33; // Seleziona il controller touch sulle sue linee dedicate.
constexpr uint8_t XPT2046_TOUCH_CLOCK_PIN = 25; // Clock generato dal firmware per la comunicazione con il touch.
constexpr uint8_t XPT2046_TOUCH_MOSI_PIN = 32; // Linea dati dall'ESP32 verso il controller touch.
constexpr uint8_t XPT2046_TOUCH_MISO_PIN = 39; // Linea dati dal controller touch verso l'ESP32; GPIO39 e solo ingresso.
constexpr uint8_t XPT2046_TOUCH_INTERRUPT_PIN = 36; // Segnale attivo quando lo schermo viene toccato; GPIO36 e solo ingresso.
constexpr uint8_t XPT2046_TOUCH_DISPLAY_ROTATION = TEMPERATURE_DISPLAY_LANDSCAPE_ROTATION; // Mantiene gli assi touch concordi con quelli del display.
constexpr uint16_t XPT2046_RAW_COORDINATE_MINIMUM = 0; // Limite inferiore grezzo del convertitore touch a 12 bit.
constexpr uint16_t XPT2046_RAW_COORDINATE_MAXIMUM = 4095; // Limite superiore grezzo del convertitore touch a 12 bit.
constexpr uint16_t SONDARIS_STARTUP_LOGO_DURATION_SECONDS = 4; // Durata della schermata iniziale, in secondi; modificare questo valore per regolarla.
constexpr uint8_t STARTUP_SUMMARY_SCREEN_DURATION_SECONDS = 5; // Tempo per leggere il riepilogo prima del controllo touch.
constexpr int16_t STARTUP_SUMMARY_TITLE_Y_PIXELS = 14; // Coordinata verticale del titolo del riepilogo, in pixel.
constexpr int16_t STARTUP_SUMMARY_TABLE_X_PIXELS = 8; // Bordo sinistro della tabella, in pixel.
constexpr int16_t STARTUP_SUMMARY_TABLE_Y_PIXELS = 46; // Bordo superiore della tabella, in pixel.
constexpr int16_t STARTUP_SUMMARY_TABLE_WIDTH_PIXELS = 304; // Larghezza della tabella, in pixel.
constexpr uint8_t STARTUP_SUMMARY_TABLE_ROW_COUNT = 4; // Righe per nome, versione, bus CN1 e stato hardware/microSD.
constexpr int16_t STARTUP_SUMMARY_TABLE_ROW_HEIGHT_PIXELS = 28; // Altezza di ogni riga della tabella, in pixel.
constexpr int16_t STARTUP_SUMMARY_TABLE_DIVIDER_X_PIXELS = 137; // Separazione tra le etichette e i valori, in pixel.
constexpr int16_t STARTUP_SUMMARY_LABEL_X_PIXELS = 22; // Inizio della colonna con i nomi dei campi, in pixel.
constexpr int16_t STARTUP_SUMMARY_VALUE_X_PIXELS = 149; // Inizio della colonna con i valori, in pixel.
constexpr int16_t STARTUP_SUMMARY_FIRST_ROW_TEXT_Y_PIXELS = 56; // Coordinata verticale del testo della prima riga, in pixel.
constexpr uint8_t STARTUP_SUMMARY_SENSOR_BUS_ROW_INDEX = 2; // Indice, da zero, della riga con l'esito OneWire/I2C.
constexpr uint8_t STARTUP_SUMMARY_HARDWARE_ROW_INDEX = 3; // Indice, da zero, della riga con lo stato microSD.
constexpr int16_t STARTUP_SUMMARY_STATUS_DETAIL_Y_PIXELS = 169; // Coordinata verticale del dettaglio SD, in pixel.
constexpr int16_t STARTUP_SUMMARY_NO_DATA_FILES_NOTE_Y_PIXELS = 190; // Coordinata verticale della nota che chiarisce che non si scrivono file dati.
constexpr int16_t STARTUP_SUMMARY_NEXT_STEP_NOTE_Y_PIXELS = 211; // Coordinata verticale dell'avviso sul successivo controllo touch.
constexpr int16_t STARTUP_SUMMARY_BOLD_TEXT_OFFSET_PIXELS = 1; // Scostamento orizzontale, in pixel, per rendere piu marcato il testo di avviso.
constexpr uint8_t STARTUP_SUMMARY_STORAGE_TEXT_BUFFER_CAPACITY = 48; // Spazio per capacità, spazio libero, unità e terminatore.
constexpr uint8_t STARTUP_SUMMARY_SENSOR_BUS_TEXT_BUFFER_CAPACITY = 48; // Spazio per modalita bus, conteggio, indirizzi e terminatore.
constexpr uint8_t STARTUP_SUMMARY_I2C_ADDRESSES_TO_DISPLAY = 2; // Indirizzi inclusi nel valore compatto della tabella di avvio.
constexpr char STARTUP_SUMMARY_SCREEN_TITLE[] = "RIEPILOGO AVVIO"; // Titolo della tabella mostrata dopo il logo.
constexpr char STARTUP_SUMMARY_NAME_LABEL[] = "Nome"; // Etichetta del nome del prodotto nella tabella.
constexpr char STARTUP_SUMMARY_VERSION_LABEL[] = "Versione"; // Etichetta della versione firmware nella tabella.
constexpr char STARTUP_SUMMARY_SENSOR_BUS_LABEL[] = "Sensori CN1"; // Etichetta del risultato generico della ricerca sul connettore.
constexpr char STARTUP_SUMMARY_HARDWARE_LABEL[] = "Hardware trovato"; // Etichetta dello stato della microSD.
constexpr char STARTUP_SUMMARY_MICRO_SD_NOT_DETECTED_VALUE[] = "NON RILEVATA"; // Valore quando la scheda non risponde al bus.
constexpr char STARTUP_SUMMARY_MICRO_SD_FILESYSTEM_ERROR_VALUE[] = "Errore FS"; // Valore quando la scheda risponde ma il filesystem non si monta.
constexpr char STARTUP_SUMMARY_MICRO_SD_DIRECTORY_PRESENT_DETAIL[] = "Cartella SondAris_Data presente"; // Conferma che la cartella dati esiste gia.
constexpr char STARTUP_SUMMARY_MICRO_SD_DIRECTORY_CREATED_DETAIL[] = "Cartella SondAris_Data creata"; // Conferma la creazione della cartella dati.
constexpr char STARTUP_SUMMARY_MICRO_SD_DIRECTORY_ERROR_DETAIL[] = "Errore cartella SondAris_Data"; // Segnala un nome occupato o una creazione non riuscita.
constexpr char STARTUP_SUMMARY_MICRO_SD_NOT_DETECTED_DETAIL[] = "Scheda assente o non comunicante"; // Cause possibili quando la scheda non risponde.
constexpr char STARTUP_SUMMARY_MICRO_SD_FILESYSTEM_ERROR_DETAIL[] = "Scheda presente, filesystem non montabile"; // Dettaglio per un errore del filesystem.
constexpr char STARTUP_SUMMARY_NO_DATA_FILES_NOTE[] = "Nessun file dati creato o modificato"; // La sola modifica possibile e la creazione della cartella dati.
constexpr char STARTUP_SUMMARY_NEXT_STEP_NOTE[] = "Segue il controllo del touch"; // Anticipa la successiva fase di avvio.
constexpr uint32_t STARTUP_TOUCH_TARGET_TIMEOUT_MILLISECONDS = 30000; // Attesa massima per ciascun bersaglio prima di offrire il recupero con BOOT.
constexpr uint32_t STARTUP_TOUCH_CAPTURE_FEEDBACK_DURATION_MILLISECONDS = 500; // Durata minima del feedback visivo dopo l'acquisizione di un punto.
constexpr uint32_t STARTUP_TOUCH_SUCCESS_MESSAGE_DURATION_MILLISECONDS = 1000; // Tempo per leggere la conferma della calibrazione completata.
constexpr uint32_t STARTUP_TOUCH_BYPASS_MESSAGE_DURATION_MILLISECONDS = 1500; // Tempo per leggere l'avviso quando si prosegue senza touch.
constexpr uint8_t STARTUP_TOUCH_CALIBRATION_TARGET_COUNT = 3; // Numero di tocchi guidati usati per calibrare e verificare il touch.
constexpr uint8_t STARTUP_TOUCH_CALIBRATION_RAW_SAMPLE_COUNT = 7; // Numero massimo di campioni grezzi mediati per ogni tocco.
constexpr uint32_t STARTUP_TOUCH_CALIBRATION_SAMPLE_INTERVAL_MILLISECONDS = 5; // Intervallo tra i campioni grezzi, in millisecondi.
constexpr uint32_t STARTUP_TOUCH_WAIT_POLL_INTERVAL_MILLISECONDS = 5; // Intervallo tra i controlli durante l'attesa di un tocco o rilascio.
constexpr int16_t STARTUP_TOUCH_CALIBRATION_TARGET_X_PIXELS[STARTUP_TOUCH_CALIBRATION_TARGET_COUNT] = {36, 284, 160}; // Coordinate X dei tre bersagli.
constexpr int16_t STARTUP_TOUCH_CALIBRATION_TARGET_Y_PIXELS[STARTUP_TOUCH_CALIBRATION_TARGET_COUNT] = {48, 48, 194}; // Coordinate Y non allineate per una calibrazione su due assi.
constexpr int16_t XPT2046_MINIMUM_ACCEPTED_RAW_COORDINATE = 50; // Rifiuta letture quasi ferme al limite inferiore del convertitore.
constexpr int16_t XPT2046_MAXIMUM_ACCEPTED_RAW_COORDINATE = 4045; // Rifiuta letture quasi ferme al limite superiore del convertitore.
constexpr float TOUCH_CALIBRATION_MINIMUM_RAW_TRIANGLE_AREA = 100000.0f; // Rifiuta tre tocchi troppo vicini o quasi allineati.

// Logo originale SondAris: astro, orbite e colori evocativi senza riprendere grafiche dei film.
constexpr uint16_t SONDARIS_LOGO_IMAGE_WIDTH_PIXELS = 320; // Larghezza del PNG di avvio e del buffer usato per decodificarne una riga.
constexpr uint16_t SONDARIS_LOGO_IMAGE_HEIGHT_PIXELS = 240; // Altezza attesa del PNG di avvio.
constexpr int16_t SONDARIS_LOGO_EMBLEM_CENTER_Y_PIXELS = 68; // Centro verticale del simbolo, in pixel.
constexpr int16_t SONDARIS_LOGO_OUTER_ORBIT_RADIUS_X_PIXELS = 96; // Semilarghezza dell'orbita esterna, in pixel.
constexpr int16_t SONDARIS_LOGO_OUTER_ORBIT_RADIUS_Y_PIXELS = 29; // Semialtezza dell'orbita esterna, in pixel.
constexpr int16_t SONDARIS_LOGO_INNER_ORBIT_RADIUS_X_PIXELS = 80; // Semilarghezza dell'orbita interna, in pixel.
constexpr int16_t SONDARIS_LOGO_INNER_ORBIT_RADIUS_Y_PIXELS = 22; // Semialtezza dell'orbita interna, in pixel.
constexpr int16_t SONDARIS_LOGO_PLANET_RADIUS_PIXELS = 28; // Raggio del pianeta stilizzato, in pixel.
constexpr int16_t SONDARIS_LOGO_WORDMARK_Y_PIXELS = 117; // Coordinata verticale del nome SondAris, in pixel.
constexpr int16_t SONDARIS_LOGO_DESCRIPTION_Y_PIXELS = 174; // Coordinata verticale della prima riga sotto il logo, in pixel.
constexpr int16_t STARTUP_BATTERY_STATUS_Y_PIXELS = 198; // Coordinata verticale della diagnostica di alimentazione sotto la descrizione.
constexpr uint8_t SONDARIS_LOGO_WORDMARK_FONT = 4; // Font TFT_eSPI per evidenziare il nome del progetto.
constexpr int16_t STARTUP_PRESENTATION_VERSION_Y_PIXELS = 222; // Coordinata verticale della versione firmware, in pixel.
constexpr uint8_t STARTUP_BATTERY_STATUS_TEXT_BUFFER_CAPACITY = 48; // Spazio per tensione, presenza e fonte di alimentazione dedotta.
constexpr char STARTUP_BATTERY_LOAD_TEST_FORMAT[] = "BAT+ %.2f V  dLoad %+.2f V"; // Mostra tensione a carico minimo e variazione al massimo carico disponibile.
constexpr char STARTUP_BATTERY_LOAD_TEST_ERROR_FORMAT[] = "BAT+ %.2f V  test carico ERR"; // Segnala che non e stato possibile applicare tutto il carico previsto.
constexpr uint16_t SONDARIS_LOGO_OUTER_ORBIT_COLOR = TFT_DARKCYAN; // Colore profondo per il percorso orbitale esterno.
constexpr uint16_t SONDARIS_LOGO_INNER_ORBIT_COLOR = TFT_CYAN; // Colore luminoso per il percorso orbitale interno.
constexpr uint16_t SONDARIS_LOGO_PLANET_COLOR = TFT_NAVY; // Colore scuro della sfera.
constexpr uint16_t SONDARIS_LOGO_PLANET_EDGE_COLOR = TFT_CYAN; // Contorno luminoso della sfera.
constexpr uint16_t SONDARIS_LOGO_ACCENT_COLOR = TFT_ORANGE; // Accento caldo per il punto di osservazione.
constexpr uint16_t SONDARIS_LOGO_WORDMARK_COLOR = TFT_CYAN; // Colore del nome SondAris.
constexpr uint16_t SONDARIS_LOGO_DESCRIPTION_COLOR = TFT_LIGHTGREY; // Colore della breve descrizione sotto il logo.
constexpr char SONDARIS_LOGO_WORDMARK[] = "SondAris"; // Nome scelto per il progetto.
constexpr char SONDARIS_LOGO_DESCRIPTION[] = "TERMOMETRO MULTIPLO"; // Prima delle due righe informative sotto il simbolo.

// Testi usati durante la verifica guidata e il recupero del touch
constexpr int16_t STARTUP_TOUCH_INSTRUCTION_Y_PIXELS = 12; // Coordinata verticale del titolo della schermata di calibrazione.
constexpr int16_t STARTUP_TOUCH_DETAIL_Y_PIXELS = 34; // Coordinata verticale dell'istruzione per il tocco corrente.
constexpr int16_t STARTUP_TOUCH_PROGRESS_Y_PIXELS = 218; // Coordinata verticale degli indicatori di avanzamento, in pixel.
constexpr int16_t STARTUP_TOUCH_BOOT_HINT_Y_PIXELS = 229; // Coordinata verticale del promemoria per escludere il touch con BOOT.
constexpr int16_t STARTUP_TOUCH_TARGET_RADIUS_PIXELS = 13; // Raggio del bersaglio circolare, in pixel.
constexpr int16_t STARTUP_TOUCH_TARGET_CROSSHAIR_HALF_SIZE_PIXELS = 18; // Mezza lunghezza delle linee della croce, in pixel.
constexpr uint8_t STARTUP_TOUCH_STEP_TITLE_BUFFER_CAPACITY = 32; // Spazio in byte per il testo "Controllo touch N/3".
constexpr uint16_t STARTUP_TOUCH_TARGET_WAITING_COLOR = TFT_YELLOW; // Colore del bersaglio in attesa del tocco.
constexpr uint16_t STARTUP_TOUCH_TARGET_DETECTED_COLOR = TFT_CYAN; // Colore del feedback mentre viene letto il tocco.
constexpr uint16_t STARTUP_TOUCH_TARGET_CAPTURED_COLOR = TFT_GREEN; // Colore del bersaglio acquisito correttamente.
constexpr uint16_t STARTUP_TOUCH_TARGET_FAILURE_COLOR = TFT_RED; // Colore usato quando il tocco non produce coordinate valide.
constexpr uint16_t STARTUP_TOUCH_TARGET_CENTER_COLOR = TFT_RED; // Colore del punto centrale del bersaglio in attesa.
constexpr uint16_t STARTUP_TOUCH_BOOT_HINT_COLOR = TFT_LIGHTGREY; // Colore del promemoria per saltare il controllo.
constexpr char STARTUP_TOUCH_STEP_TITLE_FORMAT[] = "Controllo touch %u/%u"; // Indica il numero del bersaglio corrente.
constexpr char STARTUP_TOUCH_TARGET_INSTRUCTION[] = "Tieni premuto al centro"; // Chiede di mantenere il dito sul bersaglio per acquisire i campioni.
constexpr char STARTUP_TOUCH_TARGET_DETECTED_INSTRUCTION[] = "Tocco rilevato: mantieni premuto"; // Feedback mostrato durante la lettura delle coordinate.
constexpr char STARTUP_TOUCH_TARGET_CAPTURED_INSTRUCTION[] = "Punto acquisito: rilascia il dito"; // Conferma visiva dell'acquisizione riuscita.
constexpr char STARTUP_TOUCH_TARGET_TIMEOUT_DETAIL[] = "Nessun tocco rilevato"; // Spiega l'errore quando scade l'attesa del bersaglio.
constexpr char STARTUP_TOUCH_TARGET_INVALID_DETAIL[] = "Lettura touch non valida"; // Spiega l'errore quando il controller non fornisce coordinate utilizzabili.
constexpr char STARTUP_TOUCH_TARGET_RETRY_TITLE_FORMAT[] = "Punto %u non acquisito"; // Identifica il solo bersaglio da riprovare.
constexpr char STARTUP_TOUCH_TARGET_RETRY_INSTRUCTION[] = "Tocca per ripresentare questo punto"; // Riavvia il bersaglio non acquisito senza ripetere quelli validi.
constexpr char STARTUP_TOUCH_BOOT_SKIP_HINT[] = "BOOT: salta il controllo touch"; // Rende visibile il comando per proseguire senza touch.
constexpr char STARTUP_TOUCH_CALIBRATION_SUCCESS_TITLE[] = "Touch verificato"; // Conferma che i tre punti sono stati acquisiti.
constexpr char STARTUP_TOUCH_CALIBRATION_SUCCESS_DETAIL[] = "Calibrazione pronta per questa accensione"; // Chiarisce che i dati non sono permanenti.
constexpr char STARTUP_TOUCH_CALIBRATION_RETRY_TITLE[] = "Calibrazione non valida"; // Avviso quando i tre punti non permettono una trasformazione stabile.
constexpr char STARTUP_TOUCH_CALIBRATION_RETRY_DETAIL[] = "I tre punti devono essere distinti"; // Spiega perche occorre acquisire di nuovo tutti i bersagli.
constexpr char STARTUP_TOUCH_CALIBRATION_RETRY_INSTRUCTION[] = "Tocca per ripetere tutti i punti"; // Permette di ripetere la calibrazione completa.
constexpr char STARTUP_TOUCH_CALIBRATION_BOOT_INSTRUCTION[] = "BOOT: continua senza touch"; // Recupero sempre disponibile se il touch non funziona.
constexpr char STARTUP_TOUCH_BYPASSED_TITLE[] = "Touch non verificato"; // Stato mostrato quando si sceglie il recupero con BOOT.
constexpr char STARTUP_TOUCH_BYPASSED_DETAIL[] = "Per cambiare vista usa il pulsante BOOT"; // Funzione disponibile senza calibrazione touch.

// Posizione delle righe nella vista numerica
constexpr int16_t NUMERIC_VIEW_FIRST_ROW_X_PIXELS = 12; // Margine sinistro delle righe con le temperature, in pixel.
constexpr int16_t NUMERIC_VIEW_FIRST_ROW_Y_PIXELS = 36; // Coordinata verticale della prima riga, in pixel.
constexpr int16_t NUMERIC_VIEW_ROW_SPACING_PIXELS = 22; // Distanza verticale tra due righe consecutive, in pixel.
constexpr uint8_t NUMERIC_TEMPERATURE_TEXT_BUFFER_CAPACITY = 48; // Spazio in byte per comporre una riga di temperatura.

// Posizione e scala del grafico
constexpr int16_t TEMPERATURE_GRAPH_LEFT_X_PIXELS = 42; // Bordo sinistro dell'area del grafico, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_RIGHT_X_PIXELS = 310; // Bordo destro, dove viene disegnata la misura piu recente, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_TOP_Y_PIXELS = 42; // Bordo superiore dell'area del grafico, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_BOTTOM_Y_PIXELS = 184; // Bordo inferiore dell'area del grafico, in pixel.
constexpr float TEMPERATURE_GRAPH_AXIS_MARGIN_CELSIUS = 1.0f; // Gradi aggiunti sopra e sotto i dati per non schiacciare le curve sui bordi.
constexpr int16_t TEMPERATURE_GRAPH_AXIS_LABEL_X_PIXELS = 0; // Margine sinistro delle etichette della scala in gradi Celsius, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_MINIMUM_LABEL_Y_OFFSET_PIXELS = 8; // Spostamento verso l'alto dell'etichetta minima, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_TIME_LABEL_Y_OFFSET_PIXELS = 3; // Spostamento sotto il grafico delle etichette temporali, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_NO_DATA_MESSAGE_X_PIXELS = TEMPERATURE_GRAPH_LEFT_X_PIXELS + 8; // Coordinata orizzontale del messaggio senza misure.
constexpr int16_t TEMPERATURE_GRAPH_NO_DATA_MESSAGE_Y_PIXELS = 100; // Coordinata verticale del messaggio senza misure, in pixel.
constexpr uint8_t TEMPERATURE_GRAPH_TIME_LABEL_BUFFER_CAPACITY = 16; // Spazio in byte per l'etichetta del tempo iniziale.
constexpr uint8_t TEMPERATURE_GRAPH_AXIS_LABEL_BUFFER_CAPACITY = 16; // Spazio in byte per un valore della scala in gradi Celsius.
constexpr char TEMPERATURE_GRAPH_OLDEST_SAMPLE_TIME_FORMAT[] = "-%u s"; // Forma dell'etichetta del campione piu vecchio, in secondi.
constexpr char TEMPERATURE_GRAPH_CURRENT_TIME_LABEL[] = "ora"; // Etichetta dell'istante della misura piu recente.
constexpr char TEMPERATURE_GRAPH_TEMPERATURE_LABEL_FORMAT[] = "%.0f C"; // Forma delle etichette della scala verticale.
constexpr char TEMPERATURE_GRAPH_NO_VALID_DATA_MESSAGE[] = "In attesa di misure valide"; // Messaggio mostrato prima della prima lettura valida.

// Posizione e forma della legenda del grafico
constexpr uint8_t TEMPERATURE_GRAPH_LEGEND_COLUMN_COUNT = 4; // Numero di sonde mostrate su ciascuna riga della legenda.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_COLUMN_WIDTH_PIXELS = 80; // Distanza orizzontale tra le colonne della legenda, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_FIRST_COLUMN_X_PIXELS = 0; // Coordinata orizzontale della prima colonna, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_FIRST_ROW_Y_PIXELS = 200; // Coordinata verticale della prima riga, in pixel.
constexpr int16_t TEMPERATURE_GRAPH_LEGEND_ROW_SPACING_PIXELS = 10; // Distanza verticale tra le righe della legenda, in pixel.
constexpr uint8_t TEMPERATURE_GRAPH_LEGEND_TEXT_BUFFER_CAPACITY = 20; // Spazio in byte per il nome e il valore di una sonda.
constexpr char TEMPERATURE_GRAPH_DHT_VALID_LABEL_FORMAT[] = "%s %.1f C"; // Legenda del DHT con modello e ultima temperatura valida.
constexpr char TEMPERATURE_GRAPH_DHT_UNAVAILABLE_LABEL_FORMAT[] = "%s -- C"; // Legenda del DHT quando la temperatura non e disponibile.
constexpr char TEMPERATURE_GRAPH_VALID_PROBE_LABEL_FORMAT[] = "S%u %.1f"; // Forma della voce legenda quando la misura e valida.
constexpr char TEMPERATURE_GRAPH_UNAVAILABLE_PROBE_LABEL_FORMAT[] = "S%u --"; // Forma della voce legenda quando manca la misura.

// Testi e posizione dei messaggi della schermata
constexpr int16_t TEMPERATURE_SCREEN_TITLE_X_PIXELS = 8; // Coordinata orizzontale del titolo, in pixel.
constexpr int16_t TEMPERATURE_SCREEN_TITLE_Y_PIXELS = 8; // Coordinata verticale del titolo, in pixel.
constexpr int16_t TEMPERATURE_SCREEN_NO_PROBE_X_PIXELS = 8; // Coordinata orizzontale dell'avviso quando non ci sono sonde.
constexpr int16_t TEMPERATURE_SCREEN_NO_PROBE_TITLE_Y_PIXELS = 60; // Coordinata verticale della prima riga dell'avviso.
constexpr int16_t TEMPERATURE_SCREEN_NO_PROBE_HELP_Y_PIXELS = 84; // Coordinata verticale dell'istruzione di controllo.
constexpr char TEMPERATURE_SCREEN_NUMERIC_TITLE[] = "Misure sensori"; // Titolo generico mostrato nella vista numerica.
constexpr char TEMPERATURE_SCREEN_GRAPH_TITLE[] = "Andamento temperature"; // Titolo mostrato nella vista grafica.
constexpr char TEMPERATURE_SCREEN_NO_PROBE_MESSAGE[] = "Nessun sensore utilizzabile"; // Stato generico quando non sono disponibili misure.
constexpr char TEMPERATURE_SCREEN_CHECK_PROBES_MESSAGE[] = "Verificare il collegamento"; // Istruzione generica per controllare i sensori.
constexpr char TEMPERATURE_SCREEN_SENSOR_DETECTED_MESSAGE[] = "Presenza sensore confermata"; // Conferma il rilevamento senza attribuire un modello non verificato.
constexpr char TEMPERATURE_SCREEN_SENSOR_TYPE_NOT_HANDLED_MESSAGE[] = "Tipo non ancora gestito o da confermare"; // Evita di attribuire un modello non verificato.
constexpr char TEMPERATURE_SCREEN_DHT_MODEL_REQUIRED_MESSAGE[] = "DHT rilevato: scegliere DHT11 o DHT22"; // Informa che il modello DHT deve essere indicato prima delle misure.
constexpr char TEMPERATURE_SCREEN_DHT_MEASUREMENTS_DISABLED_MESSAGE[] = "Misure disattivate finche il modello non viene scelto"; // Impedisce di interpretare i dati senza conoscere il formato.
constexpr char NUMERIC_VIEW_VALID_TEMPERATURE_FORMAT[] = "S%u: %.2f C"; // Forma di una temperatura valida nella vista numerica.
constexpr char NUMERIC_VIEW_UNAVAILABLE_TEMPERATURE_FORMAT[] = "S%u: -- (non disponibile)"; // Forma di una lettura assente nella vista numerica.
constexpr char NUMERIC_VIEW_DHT_VALID_TEMPERATURE_FORMAT[] = "%s: %.2f C"; // Temperatura DHT valida con il modello scelto dall'operatore.
constexpr char NUMERIC_VIEW_DHT_UNAVAILABLE_TEMPERATURE_FORMAT[] = "%s: -- C"; // Temperatura DHT non valida o non ancora disponibile.
constexpr char NUMERIC_VIEW_DHT_VALID_HUMIDITY_FORMAT[] = "Umidita: %.1f %%"; // Umidita relativa DHT valida, in percentuale.
constexpr char NUMERIC_VIEW_DHT_UNAVAILABLE_HUMIDITY_FORMAT[] = "Umidita: -- %"; // Umidita relativa non valida o non ancora disponibile.
constexpr int16_t NUMERIC_VIEW_DHT_TEMPERATURE_Y_PIXELS = 55; // Coordinata verticale della temperatura DHT, in pixel.
constexpr int16_t NUMERIC_VIEW_DHT_HUMIDITY_Y_PIXELS = 91; // Coordinata verticale dell'umidita DHT, in pixel.
constexpr int16_t NUMERIC_VIEW_DHT_STATUS_Y_PIXELS = 128; // Coordinata verticale dello stato del sensore, in pixel.

// Barra di controllo touch inferiore, comune alla vista numerica, al grafico e al menu tecnico.
constexpr int16_t CONTROL_BAR_X_PIXELS = 0; // Bordo sinistro della barra inferiore, in pixel.
constexpr int16_t CONTROL_BAR_WIDTH_PIXELS = TEMPERATURE_DISPLAY_WIDTH_PIXELS; // Larghezza della barra, in pixel.
constexpr int16_t CONTROL_BAR_HEIGHT_PIXELS = 14; // Altezza della barra inferiore, in pixel.
constexpr int16_t CONTROL_BAR_TOP_Y_PIXELS =
    TEMPERATURE_DISPLAY_HEIGHT_PIXELS - CONTROL_BAR_HEIGHT_PIXELS; // Prima riga della barra, in pixel.
constexpr uint16_t CONTROL_BAR_BACKGROUND_COLOR = TFT_NAVY; // Blu molto scuro usato come fondo della barra.
constexpr uint16_t CONTROL_BAR_TOP_BORDER_COLOR = TFT_DARKCYAN; // Linea sottile che separa la barra dal contenuto.
constexpr int16_t CONTROL_BAR_BUTTON_VERTICAL_INSET_PIXELS = 1; // Bordo interno superiore e inferiore dei pulsanti, in pixel.
constexpr int16_t CONTROL_BAR_BUTTON_HEIGHT_PIXELS =
    CONTROL_BAR_HEIGHT_PIXELS - 2 * CONTROL_BAR_BUTTON_VERTICAL_INSET_PIXELS; // Altezza utile dei pulsanti, in pixel.
constexpr int16_t CONTROL_BAR_BUTTON_Y_PIXELS =
    CONTROL_BAR_TOP_Y_PIXELS + CONTROL_BAR_BUTTON_VERTICAL_INSET_PIXELS; // Coordinata verticale dei pulsanti, in pixel.
constexpr int16_t CONTROL_BAR_BUTTON_CORNER_RADIUS_PIXELS = 3; // Arrotondamento degli angoli dei pulsanti, in pixel.
constexpr int16_t CONTROL_BAR_BUTTON_TEXT_Y_OFFSET_PIXELS = 2; // Margine superiore del testo nel pulsante, in pixel.
constexpr int16_t CONTROL_BAR_VIEW_BUTTON_X_PIXELS = 6; // Bordo sinistro del pulsante Valore/Andamento, in pixel.
constexpr int16_t CONTROL_BAR_VIEW_BUTTON_WIDTH_PIXELS = 96; // Larghezza del pulsante Valore/Andamento, in pixel.
constexpr int16_t CONTROL_BAR_VIEW_BUTTON_CENTER_X_PIXELS =
    CONTROL_BAR_VIEW_BUTTON_X_PIXELS + CONTROL_BAR_VIEW_BUTTON_WIDTH_PIXELS / 2; // Centro del pulsante di cambio vista, in pixel.
constexpr int16_t CONTROL_BAR_SETTINGS_BUTTON_WIDTH_PIXELS = 24; // Larghezza dell'area touch dell'ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_SETTINGS_BUTTON_RIGHT_MARGIN_PIXELS = 6; // Distanza dell'ingranaggio dal bordo destro, in pixel.
constexpr int16_t CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS =
    TEMPERATURE_DISPLAY_WIDTH_PIXELS -
    CONTROL_BAR_SETTINGS_BUTTON_RIGHT_MARGIN_PIXELS -
    CONTROL_BAR_SETTINGS_BUTTON_WIDTH_PIXELS; // Bordo sinistro del pulsante ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_SETTINGS_BUTTON_CENTER_X_PIXELS =
    CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS + CONTROL_BAR_SETTINGS_BUTTON_WIDTH_PIXELS / 2; // Centro del pulsante ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_BOOT_HELP_CENTER_X_PIXELS =
    (CONTROL_BAR_VIEW_BUTTON_X_PIXELS + CONTROL_BAR_VIEW_BUTTON_WIDTH_PIXELS +
     CONTROL_BAR_SETTINGS_BUTTON_X_PIXELS) / 2; // Centro del suggerimento BOOT tra i due pulsanti, in pixel.
constexpr int16_t CONTROL_BAR_GEAR_ICON_RADIUS_PIXELS = 3; // Raggio del corpo circolare dell'ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_GEAR_ICON_TOOTH_LENGTH_PIXELS = 2; // Lunghezza dei denti cardinali dell'ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_GEAR_ICON_TOOTH_OFFSET_PIXELS = 1; // Distanza tra il corpo e i denti dell'ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_GEAR_ICON_OUTER_RADIUS_PIXELS =
    CONTROL_BAR_GEAR_ICON_RADIUS_PIXELS + CONTROL_BAR_GEAR_ICON_TOOTH_LENGTH_PIXELS; // Raggio massimo dell'ingranaggio, in pixel.
constexpr int16_t CONTROL_BAR_GEAR_ICON_HOLE_RADIUS_PIXELS = 1; // Raggio del foro centrale dell'ingranaggio, in pixel.
constexpr uint16_t CONTROL_BAR_BUTTON_BACKGROUND_COLOR = TFT_DARKCYAN; // Colore dei pulsanti touch attivi.
constexpr uint16_t CONTROL_BAR_BUTTON_BORDER_COLOR = TFT_CYAN; // Bordo luminoso dei pulsanti touch attivi.
constexpr uint16_t CONTROL_BAR_BUTTON_TEXT_COLOR = TFT_WHITE; // Colore delle etichette dei pulsanti attivi.
constexpr uint16_t CONTROL_BAR_DISABLED_BUTTON_COLOR = TFT_DARKGREY; // Colore attenuato quando il touch non e calibrato.
constexpr uint16_t CONTROL_BAR_DISABLED_TEXT_COLOR = TFT_LIGHTGREY; // Testo attenuato dei controlli non disponibili.
constexpr uint16_t CONTROL_BAR_SETTINGS_ACTIVE_BACKGROUND_COLOR = TFT_ORANGE; // Evidenzia l'ingranaggio mentre il menu tecnico e aperto.
constexpr uint16_t CONTROL_BAR_SETTINGS_ACTIVE_ICON_COLOR = TFT_BLACK; // Contrasto dell'ingranaggio quando il menu e aperto.
constexpr uint16_t CONTROL_BAR_SETTINGS_ICON_COLOR = TFT_WHITE; // Colore dell'ingranaggio quando il menu e chiuso.
constexpr uint16_t CONTROL_BAR_BOOT_HELP_COLOR = TFT_LIGHTGREY; // Colore del suggerimento quando BOOT e l'unico comando disponibile.
constexpr char CONTROL_BAR_SWITCH_TO_GRAPH_LABEL[] = "ANDAMENTO"; // Azione mostrata quando e attiva la vista Valore.
constexpr char CONTROL_BAR_SWITCH_TO_VALUES_LABEL[] = "VALORE"; // Azione mostrata quando e attiva la vista Andamento.
constexpr char CONTROL_BAR_BOOT_HELP_LABEL[] = "BOOT: CAMBIA VISTA"; // Suggerimento mostrato se il touch non e utilizzabile.

// Contenuti e impaginazione del menu tecnico raggiunto dall'ingranaggio.
constexpr int16_t TECHNICAL_MENU_TITLE_X_PIXELS = 8; // Coordinata orizzontale del titolo, in pixel.
constexpr int16_t TECHNICAL_MENU_TITLE_Y_PIXELS = 8; // Coordinata verticale del titolo, in pixel.
constexpr int16_t TECHNICAL_MENU_VERSION_Y_PIXELS = 12; // Coordinata verticale della versione allineata a destra, in pixel.
constexpr int16_t TECHNICAL_MENU_HEADER_RULE_Y_PIXELS = 30; // Coordinata della linea sotto il titolo, in pixel.
constexpr int16_t TECHNICAL_MENU_LABEL_X_PIXELS = 12; // Inizio della colonna descrittiva, in pixel.
constexpr int16_t TECHNICAL_MENU_VALUE_RIGHT_X_PIXELS =
    TEMPERATURE_DISPLAY_WIDTH_PIXELS - 12; // Bordo destro della colonna dei valori, in pixel.
constexpr int16_t TECHNICAL_MENU_FIRST_ROW_Y_PIXELS = 42; // Coordinata verticale della prima riga informativa, in pixel.
constexpr int16_t TECHNICAL_MENU_ROW_SPACING_PIXELS = 25; // Distanza tra righe informative, in pixel.
constexpr int16_t TECHNICAL_MENU_ROW_DIVIDER_Y_OFFSET_PIXELS = 17; // Linea separatrice sotto il testo di ogni riga, in pixel.
constexpr int16_t TECHNICAL_MENU_CLOSE_HINT_Y_PIXELS = 214; // Coordinata verticale dell'istruzione per chiudere il menu, in pixel.
constexpr uint8_t TECHNICAL_MENU_I2C_ADDRESSES_TO_DISPLAY = 3; // Numero massimo di indirizzi I2C mostrati nella riga tecnica.
constexpr uint8_t TECHNICAL_MENU_I2C_ADDRESS_TEXT_BUFFER_CAPACITY = 40; // Spazio per gli indirizzi I2C visibili e il terminatore.
constexpr uint16_t TECHNICAL_MENU_TITLE_COLOR = TFT_CYAN; // Colore principale del menu tecnico.
constexpr uint16_t TECHNICAL_MENU_LABEL_COLOR = TFT_LIGHTGREY; // Colore delle descrizioni delle righe.
constexpr uint16_t TECHNICAL_MENU_VALUE_COLOR = TFT_WHITE; // Colore dei valori informativi normali.
constexpr uint16_t TECHNICAL_MENU_SUCCESS_COLOR = TFT_GREEN; // Colore degli stati pronti o correttamente calibrati.
constexpr uint16_t TECHNICAL_MENU_WARNING_COLOR = TFT_ORANGE; // Colore degli stati assenti o non disponibili.
constexpr uint16_t TECHNICAL_MENU_DIVIDER_COLOR = TFT_DARKCYAN; // Colore delle linee che organizzano le informazioni.
constexpr char TECHNICAL_MENU_TITLE[] = "MENU TECNICO"; // Titolo della schermata aperta dal pulsante ingranaggio.
constexpr char TECHNICAL_MENU_PROBE_COUNT_LABEL[] = "Sensori attivi"; // Descrizione del conteggio dei sensori utilizzabili.
constexpr char TECHNICAL_MENU_PROBE_COUNT_FORMAT[] = "%u / %u"; // Numero di sensori attivi rispetto al massimo.
constexpr char TECHNICAL_MENU_TOUCH_LABEL[] = "Touch"; // Descrizione dello stato del touchscreen.
constexpr char TECHNICAL_MENU_TOUCH_READY_VALUE[] = "CALIBRATO"; // Stato touch disponibile per questa accensione.
constexpr char TECHNICAL_MENU_TOUCH_UNAVAILABLE_VALUE[] = "NON VERIFICATO"; // Stato touch non calibrato o escluso.
constexpr char TECHNICAL_MENU_SENSOR_BUS_LABEL[] = "Sensori CN1"; // Descrizione generica del rilevamento sul connettore.
constexpr char TECHNICAL_MENU_SENSOR_BUS_ONEWIRE_FORMAT[] = "OneWire %u GPIO%u"; // Numero di dispositivi e pin dati OneWire rilevati.
constexpr char TECHNICAL_MENU_SENSOR_BUS_I2C_FORMAT[] = "I2C %u SDA%u/SCL%u"; // Conteggio e pin usati dalla scansione I2C.
constexpr char TECHNICAL_MENU_SENSOR_BUS_NO_SENSOR_VALUE[] = "NESSUN SENSORE"; // Nessun protocollo ha trovato risposte nei due tentativi.
constexpr char TECHNICAL_MENU_SENSOR_BUS_ERROR_VALUE[] = "ERRORE I2C"; // Errore durante l'inizializzazione della modalita I2C.
constexpr char TECHNICAL_MENU_SENSOR_BUS_NOT_CHECKED_VALUE[] = "NON VERIFICATO"; // Stato predefinito prima del controllo di avvio.
constexpr char TECHNICAL_MENU_I2C_ADDRESSES_LABEL[] = "Indirizzi I2C"; // Descrizione degli indirizzi che hanno risposto allo scan.
constexpr char TECHNICAL_MENU_I2C_ADDRESSES_NOT_SCANNED_VALUE[] = "NON SCANSIONATO"; // La scansione I2C non parte se OneWire o DHT trovano dispositivi.
constexpr char TECHNICAL_MENU_I2C_ADDRESSES_NONE_VALUE[] = "NESSUNO"; // Nessun indirizzo I2C ha risposto.
constexpr char TECHNICAL_MENU_I2C_ADDRESSES_ERROR_VALUE[] = "NON DISPONIBILE"; // Il bus I2C non e stato inizializzato.
constexpr char TECHNICAL_MENU_MICRO_SD_LABEL[] = "microSD"; // Descrizione dello stato della scheda.
constexpr char TECHNICAL_MENU_MICRO_SD_READY_VALUE[] = "PRONTA"; // Stato microSD montata.
constexpr char TECHNICAL_MENU_MICRO_SD_ERROR_VALUE[] = "ERRORE FS"; // Stato microSD presente ma filesystem non utilizzabile.
constexpr char TECHNICAL_MENU_MICRO_SD_ABSENT_VALUE[] = "ASSENTE"; // Stato microSD non rilevata.
constexpr char TECHNICAL_MENU_CAPACITY_LABEL[] = "Capacita - liberi"; // Descrizione dei valori di capacita e spazio residuo.
constexpr char TECHNICAL_MENU_SPACE_UNAVAILABLE_VALUE[] = "NON DISPONIBILE"; // Valore quando non si possono leggere le dimensioni della scheda.
constexpr char TECHNICAL_MENU_DIRECTORY_LABEL[] = "Cartella dati"; // Descrizione della cartella /SondAris_Data.
constexpr char TECHNICAL_MENU_DIRECTORY_PRESENT_VALUE[] = "PRESENTE"; // Stato della cartella dati gia esistente.
constexpr char TECHNICAL_MENU_DIRECTORY_CREATED_VALUE[] = "CREATA"; // Stato della cartella dati creata all'avvio.
constexpr char TECHNICAL_MENU_DIRECTORY_ERROR_VALUE[] = "ERRORE"; // Stato della cartella dati non disponibile.
constexpr char TECHNICAL_MENU_DIRECTORY_NOT_CHECKED_VALUE[] = "NON VERIFICATA"; // Stato della cartella senza filesystem utilizzabile.
constexpr char TECHNICAL_MENU_CLOSE_HINT[] = "TOCCA L'INGRANAGGIO PER CHIUDERE"; // Istruzione per tornare alla schermata temperatura.

#endif
