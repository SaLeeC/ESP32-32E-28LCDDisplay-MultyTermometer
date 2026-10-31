# Istruzioni del progetto ESP32

## Ambiente di sviluppo

- Usa PlatformIO per configurare, gestire le dipendenze, compilare, caricare il firmware e avviare il monitor seriale.
- Considera `platformio.ini` la fonte canonica per ambiente, scheda, framework, librerie e opzioni di build. Non introdurre flussi di compilazione paralleli.
- Per i comandi e le procedure d'uso, segui la sezione "Compilazione e caricamento" del [README](../README.md).

## Moduli per dispositivo

- Gestisci ogni tipo di dispositivo o periferica hardware con un modulo dedicato sotto `src/devices/`, separato da `src/main.cpp`.
- Mantieni `src/main.cpp` come punto d'ingresso e coordinamento: include gli header dei moduli e usa le loro interfacce, senza accorpare i dettagli dei singoli dispositivi.
- In C++, separa dichiarazioni e implementazione in file `.h` e `.cpp` quando appropriato. Includi gli header, non i file `.cpp`: PlatformIO compila automaticamente i sorgenti presenti sotto `src/`.
