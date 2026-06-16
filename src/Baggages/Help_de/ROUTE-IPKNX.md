### Routing IP -> KNX

Hier werden die Routing-Einstellungen für Telegramme von IP (KNX-IP-Routing, IP-Multicast) nach KNX (TP1 / EIB) vorgenommen.

#### Gruppentelegramme (Hauptgruppe 0–13 / 14–31)
Getrennt einstellbar für die Hauptgruppen 0–13 und 14–31:

- **Filtern** *(Empfehlung / Normalbetrieb)*: Nur Gruppenadressen weiterleiten, die in der **Filtertabelle** stehen. Die Filtertabelle wird von der ETS aus dem Projekt berechnet und beim (vollständigen) Programmieren in den Koppler geladen.
- **Weiterleiten**: Alle Gruppentelegramme ungefiltert weiterleiten (keine Filtertabelle nötig).
- **Blockieren**: Keine Gruppentelegramme weiterleiten.

#### Phys. adressierte Telegramme
- **Filtern (abhängig von Ziel- und Koppleradresse)** *(Empfehlung / Normalbetrieb)*: Nur Telegramme weiterleiten, deren Zieladresse topologisch zum Koppler passt.
- **Weiterleiten (nur für Testbetrieb!)**: Leitet **alle** physikalisch adressierten Telegramme ungefiltert weiter und hebelt damit die Filter-/Koppelfunktion aus. Nur zur Inbetriebnahme/Fehlersuche verwenden – im Dauerbetrieb belastet es beide Medien unnötig.
- **Blockieren**: Keine physikalisch adressierten Telegramme weiterleiten.

#### Broadcast Telegramme
- **Weiterleiten** *(Empfehlung)*: Broadcasts durchlassen. Wird u. a. für die Programmierung (Programmiermodus, Adressvergabe) benötigt.
- **Blockieren**: Broadcasts sperren. Kann linienübergreifendes Programmieren verhindern.

#### Wiederholungen (Gruppen- / phys. adressierte / Broadcast Telegramme)
Steuert, ob der Koppler ein auf der KNX-Seite (TP1) nicht erfolgreich übertragenes Telegramm **erneut sendet**:

- **Aktiviert** *(Empfehlung)*: Bei Übertragungsfehler auf dem KNX-Bus wird das Telegramm wiederholt (erhöht die Zustellsicherheit).
- **Deaktiviert**: Keine Wiederholung. Sinnvoll z. B. zur Vermeidung von Mehrfach-Telegrammen bei bewusst toleranten Anlagen oder zur Reduktion der Buslast.
