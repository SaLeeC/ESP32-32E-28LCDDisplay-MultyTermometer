# Istruzioni del progetto ESP32

## Ambiente di sviluppo

- Usa PlatformIO per configurare, gestire le dipendenze, compilare, caricare il firmware e avviare il monitor seriale.
- Considera `platformio.ini` la fonte canonica per ambiente, scheda, framework, librerie e opzioni di build. Non introdurre flussi di compilazione paralleli.
- Per i comandi e le procedure d'uso, segui la sezione "Compilazione e caricamento" del [README](../README.md).

## Moduli per dispositivo e frammenti di codice

- Gestisci ogni tipo di dispositivo o periferica hardware e ogni insieme di funzioni ricorrenti con un modulo dedicato. Tutti i file dei moduli devono stare sotto `include/`, per esempio in `include/devices/`; non collocarli in `src/`.
- Mantieni `src/main.cpp` come punto d'ingresso e coordinamento. Includi in modo esplicito, nella parte alta del file, le dichiarazioni e i frammenti di codice necessari al progetto.
- I file `.h` contengono le dichiarazioni e le interfacce. I file `.cpp` dei moduli sotto `include/` sono frammenti sorgente: includili direttamente da `src/main.cpp` con un percorso tra virgolette, per esempio `#include "devices/LCD.cpp"` per `include/devices/LCD.cpp`. Non usare un percorso con `/include/` iniziale.
- L'inclusione di un `.cpp` fa elaborare il suo contenuto come parte dell'unità di compilazione di `main.cpp`. PlatformIO compila automaticamente i `.cpp` sotto `src/`, ma non i frammenti `.cpp` sotto `include/`: non configurare questi frammenti per una compilazione separata e non duplicarli sotto `src/`.
- Per attivare o disattivare un modulo, includi o commenta il relativo `#include` in `main.cpp`. Disattiva anche le chiamate e gli altri riferimenti a quel modulo, affinché non rimangano nomi non dichiarati o funzioni mancanti. Mantieni gli header di dichiarazione prima dei frammenti che li usano e rispetta l'ordine richiesto dalle dipendenze tra frammenti.

## Nomi, dichiarazioni e commenti

- Usa nomi completi, parlanti e descrittivi (verbose) per variabili, costanti e funzioni/subroutine; evita abbreviazioni poco chiare.
- Dichiara in `include/Var.h` le variabili di stato condivise. Definisci ciascuna variabile una sola volta in un frammento `.cpp` incluso da `main.cpp`; mantieni le variabili locali nello scope in cui servono.
- Definisci le costanti di progetto in `include/Constant.h`, usando una forma compatibile con lo standard C++ configurato in PlatformIO ed evitando definizioni multiple.
- Dichiara i prototipi di funzioni/subroutine in `include/Sub.h`. Le implementazioni dei moduli devono restare sotto `include/` ed essere incluse da `main.cpp` secondo le regole sopra.
- Scrivi i commenti in italiano chiaro, rivolto anche a chi conosce solo i rudimenti di programmazione e di elettronica; non dare per scontate le conoscenze di uno specialista.
- Accompagna ogni dichiarazione con un commento che ne spieghi scopo e uso: includi variabili globali e locali, campi di strutture, costanti, tipi e funzioni. Per variabili e costanti indica unita di misura, intervalli e valori speciali quando pertinenti; per le funzioni descrivi parametri, valore restituito ed effetti collaterali.
- Nei commenti di file, funzioni e passaggi non immediati, spiega il flusso, le motivazioni delle scelte, i vincoli hardware, le condizioni di errore e le assunzioni utili a chi dovra modificare il firmware.
- Preferisci commenti generosi e concreti, ma non ripetere semplicemente cio che il codice rende gia evidente. Ogni commento deve aiutare una persona a capire o manutenere il programma e va aggiornato quando cambia il comportamento descritto.
