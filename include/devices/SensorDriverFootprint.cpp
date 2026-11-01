/*
 * Banco di collegamento usato soltanto dall'ambiente cyd-driver-footprint.
 *
 * La variabile di abilitazione resta false, quindi nessun driver interroga
 * realmente CN1. Essendo volatile, il compilatore conserva comunque nel
 * binario i costruttori e i percorsi principali di inizializzazione e lettura.
 */

namespace {

volatile bool sensorDriverFootprintCallsAreEnabled = false; // False impedisce accessi hardware nel firmware di misura.
volatile float sensorDriverFootprintValue = 0.0f; // Riceve i risultati per impedire l'eliminazione delle chiamate.

Adafruit_MCP9808 footprintMcp9808Driver; // Driver MCP9808 incluso nella misura.
Adafruit_AHTX0 footprintAhtx0Driver; // Driver condiviso da AHT10, AHT20 e DHT20.
Adafruit_Si7021 footprintSi7021Driver; // Driver Si7021 incluso nella misura.
Adafruit_HTU21DF footprintHtu21dDriver; // Driver HTU21D incluso nella misura.
ClosedCube_HDC1080 footprintHdc1080Driver(
    EXTERNAL_SENSOR_I2C_SDA_PIN,
    EXTERNAL_SENSOR_I2C_SCL_PIN); // Driver HDC1080 incluso nella misura.
Adafruit_SHT31 footprintSht3xDriver; // Driver condiviso dalla famiglia SHT3x.
Adafruit_SHT4x footprintSht4xDriver; // Driver condiviso dalla famiglia SHT4x.
TMP102 footprintTmp102Driver; // Driver SparkFun TMP102 incluso nella misura.
TMP117 footprintTmp117Driver; // Driver SparkFun TMP117 incluso nella misura.
Generic_LM75 footprintLm75Driver(
    &Wire,
    Generic_LM75::DEFAULT_I2C_ADDRESS); // Driver comune LM75 incluso nella misura.
Adafruit_AM2320 footprintAm2320Driver(&Wire); // Driver AM2320 incluso nella misura.
DHT12 footprintDht12I2cDriver(&Wire); // Driver DHT12 I2C incluso nella misura.
Adafruit_SHTC3 footprintShtc3Driver; // Driver SHTC3 incluso nella misura.
Adafruit_BMP280 footprintBmp280Driver(&Wire); // Driver BMP280 incluso nella misura.
Adafruit_BME280 footprintBme280Driver; // Driver BME280 incluso nella misura.
Bme68x footprintBme68xDriver; // Driver ufficiale Bosch per BME680 e BME688.
sensors_event_t footprintHumidityEvent = {}; // Evento riutilizzato dai driver Adafruit con API unificata.
sensors_event_t footprintTemperatureEvent = {}; // Evento temperatura riutilizzato nella misura.

}

void retainAllSensorDriverCodeForFootprintMeasurement() {
    if (!sensorDriverFootprintCallsAreEnabled) {
        return;
    }

    footprintMcp9808Driver.begin(&Wire);
    sensorDriverFootprintValue += footprintMcp9808Driver.readTempC();

    footprintAhtx0Driver.begin(&Wire);
    footprintAhtx0Driver.getEvent(
        &footprintHumidityEvent,
        &footprintTemperatureEvent);
    sensorDriverFootprintValue += footprintTemperatureEvent.temperature;

    footprintSi7021Driver.begin();
    sensorDriverFootprintValue += footprintSi7021Driver.readTemperature();
    sensorDriverFootprintValue += footprintSi7021Driver.readHumidity();

    footprintHtu21dDriver.begin(&Wire);
    sensorDriverFootprintValue += footprintHtu21dDriver.readTemperature();
    sensorDriverFootprintValue += footprintHtu21dDriver.readHumidity();

    footprintHdc1080Driver.begin(0x40);
    sensorDriverFootprintValue +=
        static_cast<float>(footprintHdc1080Driver.readTemperature());
    sensorDriverFootprintValue +=
        static_cast<float>(footprintHdc1080Driver.readHumidity());

    footprintSht3xDriver.begin(0x44);
    sensorDriverFootprintValue += footprintSht3xDriver.readTemperature();
    sensorDriverFootprintValue += footprintSht3xDriver.readHumidity();

    footprintSht4xDriver.begin(&Wire);
    footprintSht4xDriver.getEvent(
        &footprintHumidityEvent,
        &footprintTemperatureEvent);
    sensorDriverFootprintValue += footprintTemperatureEvent.temperature;

    footprintTmp102Driver.begin(0x48, Wire);
    sensorDriverFootprintValue += footprintTmp102Driver.readTempC();

    footprintTmp117Driver.begin(0x48, Wire);
    sensorDriverFootprintValue +=
        static_cast<float>(footprintTmp117Driver.readTempC());

    sensorDriverFootprintValue += footprintLm75Driver.readTemperatureC();

    footprintAm2320Driver.begin();
    sensorDriverFootprintValue += footprintAm2320Driver.readTemperature();
    sensorDriverFootprintValue += footprintAm2320Driver.readHumidity();

    footprintDht12I2cDriver.begin();
    footprintDht12I2cDriver.read();
    sensorDriverFootprintValue += footprintDht12I2cDriver.getTemperature();
    sensorDriverFootprintValue += footprintDht12I2cDriver.getHumidity();

    footprintShtc3Driver.begin(&Wire);
    footprintShtc3Driver.getEvent(
        &footprintHumidityEvent,
        &footprintTemperatureEvent);
    sensorDriverFootprintValue += footprintTemperatureEvent.temperature;

    footprintBmp280Driver.begin(0x76);
    sensorDriverFootprintValue += footprintBmp280Driver.readTemperature();
    sensorDriverFootprintValue += footprintBmp280Driver.readPressure();

    footprintBme280Driver.begin(0x76, &Wire);
    sensorDriverFootprintValue += footprintBme280Driver.readTemperature();
    sensorDriverFootprintValue += footprintBme280Driver.readHumidity();
    sensorDriverFootprintValue += footprintBme280Driver.readPressure();

    footprintBme68xDriver.begin(0x76, Wire);
    sensorDriverFootprintValue +=
        static_cast<float>(footprintBme68xDriver.fetchData());
}
