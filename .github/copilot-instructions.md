# Istruzioni del progetto ESP32

## Ambiente di sviluppo

- Usa PlatformIO per configurare, gestire le dipendenze, compilare, caricare il firmware e avviare il monitor seriale.
- Considera `platformio.ini` la fonte canonica per ambiente, scheda, framework, librerie e opzioni di build. Non introdurre flussi di compilazione paralleli.
- Per i comandi e le procedure d'uso, segui la sezione "Compilazione e caricamento" del [README](../README.md).

## Moduli per dispositivo

- Gestisci ogni tipo di dispositivo o periferica hardware con un modulo dedicato. Tutti i file appartenenti ai moduli devono stare sotto `include/`, per esempio in `include/devices/`; non collocarli in `src/`.
- Mantieni `src/main.cpp` come punto d'ingresso e coordinamento: include gli header dei moduli e usa le loro interfacce, senza accorpare i dettagli dei singoli dispositivi.
- In C++, separa dichiarazioni e implementazione quando appropriato, ma non includere direttamente file `.cpp`. PlatformIO compila automaticamente i sorgenti sotto `src/`, non quelli sotto `include/`: preferisci quindi moduli header-only in `include/`, con definizioni `inline` quando necessario. Se un modulo richiede un `.cpp` in `include/`, configura esplicitamente PlatformIO per compilarlo una sola volta.

## Nomi e dichiarazioni

- Usa nomi completi, parlanti e descrittivi (verbose) per variabili, costanti e funzioni/subroutine; evita abbreviazioni poco chiare.
- Dichiara in `include/Var.h` le variabili di stato condivise. Definisci ciascuna variabile in un solo file `.cpp`; mantieni le variabili locali nello scope in cui servono.
- Definisci le costanti di progetto in `include/Constant.h`, usando una forma compatibile con lo standard C++ configurato in PlatformIO ed evitando definizioni multiple.
- Dichiara i prototipi di funzioni/subroutine in `include/Sub.h`. Le implementazioni appartenenti a un modulo device devono restare sotto `include/`; applica le regole PlatformIO sopra per assicurarti che siano compilate.
- Accompagna ogni dichiarazione con un commento comprensibile a una persona: spiega scopo e uso; per variabili e costanti indica anche unità o intervalli quando pertinenti, e per le funzioni descrivi parametri, valore restituito ed effetti.
