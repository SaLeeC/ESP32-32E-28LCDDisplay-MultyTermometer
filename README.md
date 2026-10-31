# ESP32-32E-28LCDDisplay-MultyTermometer
Sistema di misura e registrazione delle temperature rilevate da più sonde con presentazione grafica degli andamenti

## Risorse della scheda e stato del progetto

### Scheda di riferimento

Il progetto assume la scheda gialla **ESP32-2432S028R**, con modulo
**ESP32-WROOM-32E** e display resistivo da 2,8" **ILI9341** (320×240,
orientamento orizzontale). Verificare il modello e la revisione: le varianti
con display ST7789 o cablaggi differenti possono richiedere un'altra
configurazione.

| Risorsa | Uso attuale | Possibilita o cautela |
| --- | --- | --- |
| ESP32-WROOM-32E | Esegue firmware, display e acquisizione delle temperature. | Wi-Fi e Bluetooth sono disponibili nel modulo, ma non sono usati al momento. |
| Display TFT ILI9341 | Mostra temperature numeriche e grafico. | Collegamenti TFT: MISO GPIO12, MOSI GPIO13, clock GPIO14, CS GPIO15, DC GPIO2 e retroilluminazione GPIO21. Configurazione in `platformio.ini`. |
| Touch resistivo | Non ancora inizializzato dal firmware. | E previsto come interfaccia operativa per l'operatore. Prima di attivarlo occorre verificare controller, pin e calibrazione della revisione effettiva. |
| Pulsante BOOT | GPIO0 alterna attualmente vista numerica e grafico. | GPIO0 partecipa anche alla selezione della modalita di avvio; non tenerlo premuto durante il normale reset o l'accensione. |
| Bus OneWire | GPIO27 legge fino a otto sonde DS18B20 collegate in parallelo. | Alimentare le sonde a 3,3 V e usare una sola resistenza di pull-up da 4,7 kΩ tra GPIO27 e 3,3 V. |
| Slot microSD | Non usato dal firmware. | La registrazione persistente delle misure potra essere valutata in seguito, dopo aver verificato il cablaggio della scheda. |
| Audio | Non usato dal firmware. | Disponibilita e componenti effettivi vanno controllati sulla revisione presente. |
| USB e seriale | Caricamento e monitor seriale a 115200 bit/s. | Non e necessario aggiungere una scheda custom per le funzioni attuali. |
| Alimentazione | Alimentazione USB della scheda. | La presenza di un gestore batteria non e garantita dal nome CYD: verificare lo schema e non collegare una batteria direttamente ai pin. |

Non portare 5 V sui GPIO. Usare alimentazione a tre fili per le sonde
DS18B20, non la modalita parassita, e verificare il loro pinout: i colori dei
cavi non sono standardizzati.

L'ultima compilazione PlatformIO verificata (2026-10-31) ha riportato 25.780
byte di RAM su 327.680 e 298.957 byte di Flash su 1.310.720. Sono i valori
riportati per quella build, non una misura completa della memoria dinamica
disponibile durante l'esecuzione; vanno aggiornati dopo modifiche rilevanti.

### Obiettivo del prodotto e funzioni previste

Il prodotto dovra essere semplice da usare per l'operatore, mantenendo un
processo interno strutturato e comprensibile. L'involucro previsto sara
semi-impermeabile ma non certificato IP64; per questo l'operatore dovra poter
usare il touchscreen come interfaccia, oltre al pulsante fisico quando utile.

Sono previsti questi passi, ancora da progettare e implementare:

1. Supportare piu tipi di sonde; i tipi e il modo in cui rappresentarli
   saranno definiti in seguito.
2. Presentare all'avvio una proposta grafica introduttiva, inizialmente con
   testo di dimensioni e colori diversi. Contenuto e personalizzazione sono
   da definire.
3. Guidare l'operatore con tre tocchi sullo schermo: il percorso dovra
   familiarizzarlo con l'uso del touch e permettere di verificare la
   calibrazione.

Per ora il firmware caricato usa le DS18B20, mostra valori e grafico e
permette di cambiare vista con BOOT. Touchscreen, presentazione iniziale,
guida dei tre tocchi, salvataggio su SD e altri tipi di sonde non sono ancora
implementati.

### Cablaggio attuale delle sonde

Collegare tutte le DS18B20 in parallelo sul connettore CN1:

| DS18B20 | Scheda |
| --- | --- |
| VDD | 3,3 V |
| GND | GND |
| DQ (dati) | GPIO27 |

Collegare **una resistenza da 4,7 kΩ tra GPIO27 e 3,3 V** per l'intero bus.
GPIO35 è solo ingresso e GPIO21 controlla la retroilluminazione: non sono
sostituti di GPIO27.

## Compilazione e caricamento

Installare [PlatformIO](https://platformio.org/) (CLI o estensione VS Code).
Nella directory del repository:

```sh
pio run -e cyd
pio run -e cyd -t upload
pio device monitor -b 115200
```

`platformio.ini` fissa piattaforma e librerie e configura i pin TFT per tutte
le unità di compilazione, senza modificare i file delle librerie.
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
  le sonde, il pulsante BOOT e il display.

In `main.cpp` gli include vanno nell'ordine: librerie, `Constant.h`, `Var.h`,
`Sub.h` e poi i frammenti `.cpp` dei moduli attivi. Questi frammenti vengono
inseriti nel programma principale dal preprocessore: PlatformIO non li compila
una seconda volta. Per disattivare un modulo si commenta il suo include e si
disabilitano anche le chiamate che lo usano. Ogni frammento va incluso una sola
volta, per evitare definizioni duplicate.

## Utilizzo

All'avvio vengono identificate fino a otto DS18B20: il monitor seriale associa
le etichette S1…S8 agli indirizzi ROM completi. L'associazione resta invariata
durante la sessione; aggiungendo o rimuovendo sonde e riavviando può cambiare.
Le sonde oltre il limite sono ignorate con un avviso seriale.

Premere brevemente **BOOT** per alternare:

1. **Valori numerici**, una riga colorata per ciascuna sonda.
2. **Grafico temporale**, una curva colorata per sonda, legenda e scala
   verticale automatica in °C; la misura più recente è a destra.

La misura avviene ogni secondo, con conversione simultanea a 12 bit e attesa
non bloccante di 750 ms. Il grafico conserva gli ultimi 120 campioni in RAM
(circa due minuti), persi al reset: non viene creato un file sulla SD.
Le letture assenti o non valide appaiono come `--` e interrompono la curva,
senza falsare la scala. Senza sonde appare un messaggio di cablaggio.
Le sonde vengono cercate solo all'avvio: dopo averne aggiunte, riavviare.
Non tenere BOOT premuto durante accensione/reset nell'uso normale:
GPIO0 seleziona anche il bootloader dell'ESP32.

### Verifica sulla scheda

Non è presente un'infrastruttura di test automatici. Dopo la compilazione:

- Collegare almeno due sonde e verificare ROM distinti e valori plausibili.
- Scaldare una sola sonda: devono cambiare il suo valore e la sua curva.
- Cambiare vista durante la conversione e tenere BOOT premuto: deve esserci
  un solo cambio per pressione, senza fermare l'acquisizione.
- Scollegare una sonda: deve apparire `--`, con un'interruzione nel grafico;
  ricollegare la stessa sonda e verificare la ripresa.
- Attendere oltre due minuti per verificare lo scorrimento dello storico;
  riavviare senza sonde per verificare il messaggio dedicato.

Configurazione hardware di riferimento:
[ESP32 Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display).
