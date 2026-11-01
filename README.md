# SondAris
Sistema di misura e registrazione delle temperature rilevate da più sonde con presentazione grafica degli andamenti

## Risorse della scheda e stato del progetto

### Scheda di riferimento

Il progetto assume la scheda gialla **ESP32-2432S028R**, con modulo
**ESP32-WROOM-32E** e display resistivo da 2,8" **ILI9341** (320×240,
orientamento orizzontale). Verificare il modello e la revisione: le varianti
con display ST7789 o cablaggi differenti possono richiedere un'altra
configurazione.

Wiki della scheda di riferimento: [2.8inch ESP32-32E Display - LCD wiki](https://www.lcdwiki.com/2.8inch_ESP32-32E_Display).

| Risorsa | Uso attuale | Possibilita o cautela |
| --- | --- | --- |
| ESP32-WROOM-32E | Esegue firmware, display e acquisizione delle temperature. | Wi-Fi e Bluetooth sono disponibili nel modulo, ma non sono usati al momento. |
| Flash interna e OTA | Due partizioni applicazione uguali da 2.031.616 byte predispongono l'alternanza OTA e una futura logica di recupero. NVS e coredump restano disponibili. | L'aggiornamento via Wi-Fi deve ancora essere implementato. Non esiste una partizione filesystem interna: i dati permanenti devono usare la microSD. |
| Display TFT ILI9341 | Mostra temperature numeriche e grafico. | Collegamenti TFT: MISO GPIO12, MOSI GPIO13, clock GPIO14, CS GPIO15, DC GPIO2 e retroilluminazione GPIO21. Configurazione in `platformio.ini`. |
| Touch resistivo XPT2046 | La libreria `bodmer/TFT_Touch` gestisce la comunicazione SPI software sulle linee GPIO dedicate; una calibrazione affine a tre punti viene acquisita a ogni avvio. | Clock GPIO25, MOSI GPIO32, MISO GPIO39, CS GPIO33 e interrupt GPIO36. La calibrazione resta in RAM; verificare pin e revisione della scheda. |
| Pulsante BOOT | GPIO0 alterna la vista numerica e il grafico; permette anche di proseguire se il touch non e verificato. | GPIO0 partecipa alla selezione della modalita di avvio; non tenerlo premuto durante il normale reset o l'accensione. |
| Connettore CN1 | OneWire e DHT usano GPIO27 nel ciclo standard e GPIO22 nel ciclo alternativo; I²C prova SDA/SCL GPIO27/22 e poi GPIO22/27. | La ricerca ripete l'intera sequenza con i pin scambiati solo se il primo ciclo non rileva dispositivi; resta attiva una sola modalità. |
| Sonde DS18B20 | In modalita OneWire, il GPIO dati attivo legge fino a otto sonde collegate in parallelo. | Alimentare le sonde a 3,3 V e usare una sola resistenza di pull-up da 4,7 kΩ tra il GPIO dati attivo e 3,3 V. |
| Sensori DHT11/DHT22 | Un sensore DHT usa come filo dati il GPIO single-wire del ciclo che lo rileva: GPIO27 oppure GPIO22. | Il firmware riconosce il protocollo ma non il modello: l'operatore sceglie DHT11 o DHT22 sul touch prima di attivare le misure. |
| Sensori I²C esterni | Il firmware scandisce gli indirizzi standard da 0x08 a 0x77 a 100 kHz con entrambe le mappature SDA/SCL. | Usare logica a 3,3 V e verificare la presenza di pull-up su SDA e SCL; non collegare contemporaneamente dispositivi OneWire, DHT e I²C a CN1. |
| Slot microSD | All'avvio il firmware verifica il filesystem, mostra capacita e spazio residuo e prepara `/SondAris_Data` se manca. | Pin SPI: CS GPIO5, clock GPIO18, MISO GPIO19 e MOSI GPIO23. Non formatta e non crea file dati; la registrazione delle misure resta da progettare. |
| Audio | Non usato dal firmware. | Disponibilita e componenti effettivi vanno controllati sulla revisione presente. |
| USB e seriale | Caricamento e monitor seriale a 115200 bit/s. | Non e necessario aggiungere una scheda custom per le funzioni attuali. |
| Alimentazione e batteria | Il connettore da 1,25 mm accetta una LiPo da 3,7 V; il circuito TP4054 la ricarica da USB. All'accensione il firmware confronta GPIO34 con retroilluminazione, LED RGB e Wi-Fi spenti, poi con retroilluminazione al massimo, tutti i canali RGB accesi e scansione Wi-Fi a 19,5 dBm senza risparmio energetico. | Il rosso del LED RGB condivide GPIO22 con CN1: viene acceso solo durante il test e rilasciato prima della ricerca sensori. La risposta al massimo salto di carico disponibile aggiunge un dato sperimentale, ma non viene ancora usata da sola per dichiarare la presenza della batteria o stimarne la capacità. |

Non portare 5 V sui GPIO. Usare alimentazione a tre fili per le sonde
DS18B20, non la modalita parassita, e verificare il loro pinout: i colori dei
cavi non sono standardizzati.

La Flash interna privilegia il firmware con due partizioni OTA uguali e della
massima dimensione pratica; non riserva un filesystem, perche configurazioni e
diagnostica usano NVS/coredump mentre i dati applicativi sono affidati alla microSD.

L'ultima compilazione PlatformIO verificata (2026-11-01) della versione v1.034
ha riportato 50.828 byte di RAM su 327.680 e 914.253 byte di Flash su 2.031.616.
Sono i valori del riepilogo del build, non una misura completa della memoria
dinamica disponibile durante l'esecuzione.
La versione del firmware e **v1.034 01-11-2026**; il valore canonico e in
`include/Constant.h` e deve restare coerente con l'intestazione di `main.cpp`
e la schermata di avvio.

### Obiettivo del prodotto e funzioni previste

Il prodotto dovra essere semplice da usare per l'operatore, mantenendo un
processo interno strutturato e comprensibile. L'involucro previsto sara
semi-impermeabile ma non certificato IP64; il touchscreen e quindi una delle
interfacce operative, con BOOT come alternativa fisica.

Sono presenti il logo SondAris all'avvio, con due righe informative e una
durata configurabile in secondi in `include/Constant.h`, la guida a tre
bersagli e la calibrazione affine del touch XPT2046. I tre tocchi vengono
acquisiti a ogni accensione e la trasformazione rimane solo in RAM. Se il
touch non risponde entro 30 secondi o i punti non sono utilizzabili,
l'operatore puo riprovare l'intera sequenza oppure premere BOOT e continuare
senza touch. Dopo una calibrazione valida, un pulsante disegnato sul display
alterna le viste come BOOT.

Il PNG 320x240 in `logo/SondAris_320x240.png` e la sorgente del logo mostrato
all'avvio. Uno script PlatformIO genera i dati compressi nella cartella di
build e PNGdec li disegna riga per riga: non occorre caricare immagini su SD.

Restano da completare il riconoscimento e l'acquisizione adattiva dei modelli
elencati nel catalogo sensori, l'eventuale personalizzazione dei testi sotto
il logo e la registrazione permanente dei dati su SD.

## Catalogo dei sensori

Questi sono tutti i modelli attualmente censiti nei cataloghi firmware di
`include/Constant.h`. Le voci definiscono i sensori da considerare e i relativi
riferimenti ai driver; non significano che ogni modello sia gia identificato e
letto dal programma durante l'acquisizione.

- **I²C:** `MCP9808`, `AHT10`, `AHT20`, `DHT20`, `Si7021`, `HTU21D`,
  `HDC1080`, `SHT30`, `SHT31`, `SHT35`, `SHT40`, `SHT41`, `SHT45`,
  `TMP102`, `TMP117`, `LM75`, `LM75A`, `LM75B`, `TMP75`, `TMP75B`,
  `TMP175`, `TMP275`, `PCT2075`, `TCN75A`, `MCP9800`, `MCP9801`,
  `MCP9802`, `MCP9803`, `STLM75`, `STDS75`, `ADT75`, `DS75`, `DS7505`,
  `AM2320`, `DHT12` (modalita I²C), `SHTC3`, `BMP280`, `BME280`,
  `BME680` e `BME688`.
- **Dallas/Maxim OneWire:** `DS18S20`, `DS1822`, `MAX31826`, `DS18B20`,
  `MAX31820`, `DS1825`, `MAX31825`, `DS1923 Hygrochron` e `DS28EA00`.
- **DHT single-wire proprietario:** `DHT11`, `DHT12`, `DHT21` (`AM2301`)
  e `DHT22` (`AM2302`). Il `DHT12` compare anche nel gruppo I²C perche
  supporta entrambe le modalita.

### Stato dell'integrazione

- **OneWire:** il firmware registra i dispositivi e confronta i codici
  famiglia con il catalogo. L'acquisizione corrente usa il driver
  DallasTemperature per il codice famiglia `0x28` (condiviso da `DS18B20` e
  `MAX31820`, che non vengono distinti tra loro). Le altre voci non sono
  ancora abilitate per la misura; `MAX31825`, `MAX31826` e `DS1923` richiedono
  adattatori o driver specifici.
- **DHT:** il firmware verifica il frame e il checksum, ma non ricava da questi
  il modello. Sono selezionabili e leggibili `DHT11` e `DHT22`; `DHT12` in
  single-wire e `DHT21` non sono ancora integrati nel percorso di misura.
- **I²C:** la scansione rileva gli indirizzi e li associa ai modelli candidati
  del catalogo, ma non identifica ancora il modello effettivo ne avvia la
  relativa acquisizione. La scelta manuale futura per i dispositivi LM75
  ambigui e annotata nel ToDo.

## ToDo

1. Valutare un menu di scelta manuale per i sensori della famiglia LM75 che
   rispondono correttamente al protocollo comune ma non espongono informazioni
   sufficienti per identificarne con certezza il modello. Nella prima fase
   saranno caricati soltanto i modelli riconosciuti automaticamente; i
   dispositivi rimasti ambigui non entreranno nella lista dei sensori attivi.

### Cablaggio del connettore CN1

In modalita OneWire, collegare le DS18B20 in parallelo:

| DS18B20 | Scheda |
| --- | --- |
| VDD | 3,3 V |
| GND | GND |
| DQ (dati) | GPIO27 |

Collegare **una resistenza da 4,7 kΩ tra GPIO27 e 3,3 V** per l'intero bus.
GPIO35 è solo ingresso e GPIO21 controlla la retroilluminazione: non sono
sostituti di GPIO27.

In modalita I²C alternativa:

| Segnale del sensore | Scheda |
| --- | --- |
| SDA | GPIO27 |
| SCL | GPIO22 |
| VCC | 3,3 V |
| GND | GND |

Verificare che SDA e SCL abbiano pull-up verso 3,3 V, integrati nel modulo o
esterni. Collegare un solo tipo di bus a CN1: almeno un dispositivo OneWire
rilevato all'avvio mantiene la modalita OneWire e impedisce il rilevamento DHT
e lo scan I²C.

In modalita DHT single-wire, collegare un solo DHT11 o DHT22:

| DHT | Scheda |
| --- | --- |
| DATA | GPIO27 |
| VCC | 3,3 V |
| GND | GND |

GPIO22 non viene usato in questa modalita. I moduli DHT spesso includono una
resistenza di pull-up; per un sensore nudo collegare una resistenza tra DATA e
3,3 V (tipicamente 4,7–10 kΩ). Non collegare insieme dispositivi DHT, OneWire
e I²C sul connettore CN1.

## Compilazione e caricamento

Installare [PlatformIO](https://platformio.org/) (CLI o estensione VS Code).
Nella directory del repository:

```sh
pio run -e cyd
pio run -e cyd -t upload
pio device monitor -b 115200
```

L'ambiente `cyd-driver-footprint` serve soltanto a stimare l'ingombro dei
driver I²C selezionati: forza nel binario i principali percorsi di
inizializzazione e lettura, ma non interroga i sensori. Non deve essere usato
per il normale caricamento sulla scheda.

```sh
pio run -e cyd-driver-footprint
```

La misura verificata ha prodotto 51.516 byte di RAM e 943.605 byte di Flash:
rispetto al firmware normale, l'insieme dei driver aggiunge 688 byte di RAM e
29.352 byte di Flash. Il risultato conferma che i driver occupano una parte
ridotta di ciascuno slot OTA; non rappresenta ancora il costo della futura
logica adattiva di identificazione e acquisizione.

`platformio.ini` fissa piattaforma e librerie e configura i pin TFT per tutte
le unità di compilazione, senza modificare i file delle librerie. La tabella
`partitions_ota_no_filesystem.csv` assegna due slot applicazione da
2.031.616 byte per predisporre l'alternanza OTA, conserva NVS e coredump e
non riserva spazio a SPIFFS, LittleFS o FAT interni. La microSD resta l'unico
filesystem previsto per i dati applicativi; trasporto Wi-Fi e recupero OTA
saranno implementati separatamente.
Se necessario specificare la porta con `--upload-port` o `--port` nel monitor.
Se il caricamento non parte, tenere premuto BOOT durante il reset e rilasciarlo
quando inizia il trasferimento.

## Organizzazione del firmware

Il firmware e composto da `src/main.cpp` e da moduli inclusi esplicitamente
dal programma principale. `main.cpp` coordina l'avvio e il ciclo operativo;
non contiene i dettagli di ogni periferica.

- `include/Constant.h` raccoglie pin, tempi, limiti, colori e parametri grafici.
- `include/Var.h` raccoglie lo stato condiviso e gli oggetti globali usati dai moduli.
- `include/Sub.h` elenca le funzioni che i moduli rendono disponibili.
- `include/devices/` contiene un frammento `.cpp` per ciascuna periferica, come
  scanner CN1, sonde, pulsante BOOT e display.

In `main.cpp` gli include vanno nell'ordine: librerie, `Constant.h`, `Var.h`,
`Sub.h` e poi i frammenti `.cpp` dei moduli attivi. Questi frammenti vengono
inseriti nel programma principale dal preprocessore: PlatformIO non li compila
una seconda volta. Per disattivare un modulo si commenta il suo include e si
disabilitano anche le chiamate che lo usano. Ogni frammento va incluso una sola
volta, per evitare definizioni duplicate.

## Utilizzo

La ricerca su CN1 esegue in ordine OneWire, DHT e I²C con due mappature:

1. **Standard:** OneWire/DHT su GPIO27; I²C SDA su GPIO27 e SCL su GPIO22.
2. **Scambiata:** OneWire/DHT su GPIO22; I²C SDA su GPIO22 e SCL su GPIO27.

Il secondo ciclo parte solo se il primo non rileva alcun dispositivo. Per ogni
mappatura, un dispositivo OneWire interrompe la ricerca degli altri protocolli;
se OneWire e vuoto, il firmware invia il segnale di avvio DHT e controlla frame
completo e checksum; se anche il DHT non risponde, esegue lo scan I²C a 100 kHz
degli indirizzi standard 0x08–0x77. Il primo protocollo che rileva dispositivi
resta l'unica modalita attiva. Le mappature effettivamente provate e quella
selezionata sono riportate nel riepilogo di avvio e nel menu tecnico.

La ricerca CN1 avviene prima del logo e del controllo microSD. Con zero sonde
riconosciute e utilizzabili il processo di acquisizione resta esplicitamente
fermo. Appena almeno una sonda e configurata, il processo parte; logo,
riepilogo e attese dell'interfaccia continuano a servire le misure. Questa
scelta e indipendente dalla presenza della microSD: buffer e salvataggio CSV
restano da definire separatamente.

In modalita OneWire il monitor seriale associa le etichette S1…S8 agli
indirizzi ROM completi. Fino a otto dispositivi OneWire sono registrati con le
famiglie candidate; i dispositivi non riconosciuti non vengono misurati. Le
sonde oltre il limite sono ignorate con un avviso seriale.

Il protocollo DHT non distingue automaticamente DHT11 e DHT22: dopo la
calibrazione touch compare una schermata di scelta. Solo dopo la selezione
esplicita il firmware inizializza il driver sul GPIO che ha rilevato il
sensore e attiva le misure. Se l'operatore preme BOOT, il touch non e calibrato
o scade l'attesa, il modello resta non selezionato e le misure rimangono
disattivate. Il DHT mostra temperatura e umidita nella vista numerica e
temperatura nel grafico, con un campione ogni due secondi.

Se entrambi i cicli terminano senza dispositivi, dopo la calibrazione appare
la schermata **“Nessun sensore rilevato. Collegate dei sensori e premete
Start”**. Il pulsante Start ripete entrambi i cicli; BOOT offre la stessa
funzione fisica se il touch non e calibrato. Durante il tentativo viene
mostrato un feedback di ricerca e, se un dispositivo viene trovato, il
riepilogo viene aggiornato. Se il dispositivo e un DHT, viene quindi proposta
la scelta DHT11/DHT22.

La scansione I²C resta attiva per la sessione se trova almeno un dispositivo.
L'indirizzo e il numero di dispositivi sono visibili nel riepilogo e nel menu
tecnico; il firmware registra inoltre fino a otto indirizzi I²C in `Var.h`.
Ogni voce mantiene i riferimenti alle famiglie compatibili del catalogo in
`Constant.h`. Un indirizzo può corrispondere a più famiglie: il riferimento
identificato resta vuoto finché non viene verificato il protocollo specifico.
Gli indirizzi senza corrispondenza restano registrati come dispositivi non
catalogati. Se lo scan trova più di otto dispositivi, vengono conservati nel
registro i primi otto e la scansione continua.
In modalita OneWire, un registro separato conserva fino a otto ROM completi,
associa a ciascun codice famiglia tutte le voci candidate di `ONE_WIRE_SENSOR_CATALOG`
e lascia vuoto il riferimento identificato finché non viene verificato il
modello specifico. L'elenco delle DS18B20 ancora gestite per la misura resta
separato dal registro generale.

La schermata di avvio mostra il logo **SondAris** per 4 secondi (durata
modificabile in `include/Constant.h`), poi una tabella di riepilogo con nome,
versione **v1.029 01-11-2026**, esito dei sensori CN1 e risultato della verifica
microSD. La tabella
resta visibile per 5 secondi, configurabili in `include/Constant.h`. Se serve,
il firmware prepara `/SondAris_Data`, senza creare file di dati. Poi guida
l'operatore attraverso tre bersagli. Tenere il dito al centro finche il
bersaglio diventa verde, poi rilasciarlo: il giallo indica che il punto e in
attesa, il ciano che il tocco e stato rilevato e il rosso una lettura non
valida. Se un singolo punto non viene acquisito, si puo riprovare quel punto
senza ripetere quelli gia validi; se la geometria complessiva non e valida,
vengono richiesti di nuovo tutti e tre. BOOT e indicato in ogni schermata e
consente di escludere subito il touch. La calibrazione viene ricalcolata a ogni
accensione e non e salvata in modo permanente.

La tabella indica capacità totale e spazio residuo usando la stessa unità
(B, KB, MB, GB o TB). Con almeno 100 MB liberi la cella microSD ha sfondo
verde; sotto la soglia diventa rossa e il testo in grassetto evidenzia
l'allerta. Se la scheda risponde ma il filesystem non e montabile, compare
**Errore FS** su fondo rosso; se la scheda non risponde, compare **NON
RILEVATA**.

Quando il filesystem e montabile, all'avvio il firmware controlla la cartella
`/SondAris_Data`: se esiste la segnala come presente, altrimenti la crea e
mostra la conferma. Se il nome e gia occupato da un file o la creazione fallisce,
compare un avviso. Questa e l'unica scrittura eseguita sulla microSD: non vengono
creati file di dati e la scheda non viene mai formattata.

Premere brevemente **BOOT** per alternare:

1. **Valori numerici**, una riga colorata per ciascuna sonda.
2. **Grafico temporale**, una curva colorata per sonda, legenda e scala
   verticale automatica in °C; la misura più recente è a destra.

La parte inferiore dello schermo ospita una barra di controllo alta 14 pixel.
Il pulsante a sinistra mostra l'azione successiva (**Andamento** o **Valore**);
quello a destra, con l'ingranaggio, apre e chiude il **menu tecnico**, che
riporta versione, sensori attivi, stato del touch, modalita e pin CN1,
indirizzi I²C trovati e informazioni microSD. Se il
touch non e calibrato, i comandi touch appaiono attenuati e la barra indica che
BOOT consente comunque di cambiare vista. Nel menu tecnico, il pulsante sinistro
o BOOT chiude il menu e torna alla vista scelta.

Le DS18B20 vengono misurate ogni secondo, con conversione simultanea a 12 bit
e attesa non bloccante di 750 ms. DHT11 e DHT22 vengono letti ogni due secondi;
la vista numerica mostra temperatura e umidita, mentre il grafico traccia la
temperatura. Lo storico conserva 120 campioni in RAM (circa due minuti per
OneWire o quattro minuti per DHT), persi al reset: non viene creato un file SD.
Le letture assenti o non valide appaiono come `--` e interrompono la curva,
senza falsare la scala. Se un bus risponde ma il tipo non e gestito o resta da
confermare, il display indica che la presenza del sensore e stata verificata
ma che non sono disponibili misure. Se nessun dispositivo risponde, appare la
schermata Start descritta sopra. La ricerca si ripete dopo aver premuto Start
o BOOT; dopo aver aggiunto sensori durante il funzionamento, riavviare o usare
la schermata di retry quando e visibile.
Non tenere BOOT premuto durante accensione/reset nell'uso normale:
GPIO0 seleziona anche il bootloader dell'ESP32.

### Verifica sulla scheda

Non è presente un'infrastruttura di test automatici. Dopo la compilazione:

- Collegare almeno due sonde e verificare ROM distinti e valori plausibili.
- Scaldare una sola sonda: devono cambiare il suo valore e la sua curva.
- Cambiare vista durante la conversione e tenere BOOT premuto: deve esserci
  un solo cambio per pressione, senza fermare l'acquisizione.
- Verificare che il riepilogo mostri la versione `v1.034 01-11-2026`, il
  risultato dei sensori CN1, la mappatura GPIO e lo stato microSD prima dei tre bersagli touch;
  mantenere ogni tocco finche il
  bersaglio diventa verde, quindi rilasciare e verificare l'avanzamento.
- Con almeno una DS18B20 collegata, verificare che venga indicata la modalita
  OneWire, che gli indirizzi I²C risultino non scansionati e che le misure
  proseguano durante logo, riepilogo e calibrazione touch.
- Collegare separatamente un sensore OneWire, un DHT e un dispositivo I²C al
  GPIO22: verificare che il ciclo con pin scambiati identifichi il protocollo
  corretto e mostri la mappatura nel riepilogo/menu.
- Con un dispositivo I²C su GPIO27/22 e poi su GPIO22/27, verificare che il
  riepilogo mostri SDA/SCL e gli indirizzi trovati.
- Senza alcun dispositivo collegato a CN1, verificare che vengano provate
  entrambe le mappature e compaia “Nessun sensore rilevato. Collegate dei
  sensori e premete Start”; collegare un sensore e premere Start, quindi
  verificare il nuovo rilevamento. Ripetere usando BOOT come alternativa e
  controllare che, prima del rilevamento, non partano acquisizioni.
- Collegare un DHT11 e poi un DHT22 (separatamente): verificare il rilevamento,
  la scelta esplicita del modello e le letture di temperatura e umidita.
- Durante la scelta DHT premere BOOT, lasciare scadere il tempo e ripetere
  bypassando la calibrazione touch: in tutti i casi non devono partire misure.
- Provare un bersaglio che non viene acquisito: la schermata deve indicare il
  motivo, consentire di ripresentare solo quel punto e mostrare sempre il
  comando BOOT per proseguire senza touch.
- Toccare l'ingranaggio, verificare le informazioni del menu tecnico e toccarlo
  di nuovo per tornare alla schermata temperatura.
- Con la microSD inserita, verificare capacità, spazio libero e sfondo verde
  quando restano almeno 100 MB; controllare che `/SondAris_Data` risulti
  presente oppure venga creata e segnalata. Nessun file dati deve essere scritto.
- Lasciare scadere l'attesa di un bersaglio e verificare sia il tentativo
  touch di ripetizione sia il recupero con BOOT; ripetere anche con il touch
  non collegato per controllare il fallback all'avvio.
- Scollegare una sonda: deve apparire `--`, con un'interruzione nel grafico;
  ricollegare la stessa sonda e verificare la ripresa.
- Attendere oltre due minuti per verificare lo scorrimento dello storico;
  avviare senza dispositivi per verificare la schermata Start e il retry.

Configurazione hardware di riferimento:
[ESP32 Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display).
