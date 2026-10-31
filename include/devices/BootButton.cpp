/*
 * Modulo del pulsante BOOT.
 *
 * GPIO0 e mantenuto normalmente alto dalla resistenza interna e scende a LOW
 * quando si preme il pulsante. Il contatto puo oscillare per pochi millisecondi:
 * si accetta il nuovo stato solo dopo che e rimasto stabile abbastanza a lungo.
 * Una pressione stabile alterna la vista numerica e quella grafica.
 */

void initializeBootButton() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); // Evita un livello elettrico indefinito quando il pulsante non e premuto.
}

void updateBootButton(uint32_t currentTimeMilliseconds) {
    static bool lastRawButtonLevel = HIGH; // Ultimo livello elettrico letto, conservato tra un ciclo e il successivo.
    static bool stableButtonLevel = HIGH; // Ultimo livello accettato dopo il controllo anti-rimbalzo.
    static uint32_t rawButtonLevelChangeTimeMilliseconds = 0; // Istante dell'ultima variazione del livello grezzo.
    const bool currentRawButtonLevel = digitalRead(BOOT_BUTTON_PIN); // LOW indica una pressione perche il pin usa INPUT_PULLUP.

    if (currentRawButtonLevel != lastRawButtonLevel) {
        lastRawButtonLevel = currentRawButtonLevel;
        rawButtonLevelChangeTimeMilliseconds = currentTimeMilliseconds; // Ricomincia l'attesa se il contatto continua a oscillare.
    }

    if (currentRawButtonLevel != stableButtonLevel &&
        static_cast<uint32_t>(currentTimeMilliseconds - rawButtonLevelChangeTimeMilliseconds) >=
            BOOT_BUTTON_DEBOUNCE_DURATION_MILLISECONDS) {
        stableButtonLevel = currentRawButtonLevel;

        if (stableButtonLevel == LOW) {
            // Si cambia vista solo alla pressione; il rilascio non produce un secondo cambio.
            graphViewIsActive = !graphViewIsActive;
            temperatureDisplayNeedsRedraw = true;
        }
    }
}
