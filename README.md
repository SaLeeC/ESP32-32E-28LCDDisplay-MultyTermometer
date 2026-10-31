# ESP32-32E-28LCDDisplay-MultyTermometer
Sistema di misura e registrazione delle temperature rilevate da più sonde con presentazione grafica degli andamenti

## Hardware supportato

Il firmware usa la scheda gialla **ESP32-2432S028R**, con ESP32-WROOM-32E,
display resistivo da 2,8" **ILI9341** (320×240 in orizzontale).
Verificare il modello prima dell'acquisto: le varianti ST7789, incluse alcune
schede con due porte USB, richiedono una diversa configurazione del display.
La presenza di un gestore batteria non è garantita dal nome CYD: verificare
lo schema della propria revisione e non collegare una batteria direttamente
ai pin di alimentazione.

Non servono schede custom. Oltre alla scheda, un cavo/alimentatore USB e i
cavetti, occorrono da una a **otto sonde DS18B20** e una resistenza da **4,7 kΩ**.
Touchscreen, SD e audio restano disponibili sull'hardware ma non sono usati
da questo firmware.

### Cablaggio delle sonde

Collegare tutte le sonde in parallelo sul connettore CN1:

| DS18B20 | Scheda |
| --- | --- |
| VDD | 3,3 V |
| GND | GND |
| DQ (dati) | GPIO27 |

Collegare **una resistenza da 4,7 kΩ tra GPIO27 e 3,3 V** per l'intero bus.
Usare alimentazione a tre fili, non la modalità parassita; non portare 5 V
sul GPIO. Verificare il pinout delle sonde: i colori dei cavi non sono
standardizzati. GPIO35 è solo ingresso e GPIO21 controlla la retroilluminazione:
non sono sostituti di GPIO27.

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
