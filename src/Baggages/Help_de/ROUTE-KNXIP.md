### Routing KNX -> IP

Hier werden die Routing-Einstellungen für Telegramme von KNX (TP1 / EIB) nach IP (KNX-IP-Routing, IP-Multicast) vorgenommen.

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

#### Bestätigung (ACK) von Gruppentelegrammen
- **immer**: Der Koppler bestätigt jedes Gruppentelegramm auf der KNX-Seite.
- **nur bei Weiterleitung** *(Empfehlung)*: Bestätigung nur, wenn das Telegramm tatsächlich weitergeleitet (gefiltert durchgelassen) wird.

#### Bestätigung (ACK) von phys. adressierten Telegrammen
- **nur bei Weiterleitung** *(Empfehlung)*: Bestätigung nur bei tatsächlicher Weiterleitung.
- **immer**: Jedes phys. adressierte Telegramm wird bestätigt.
- **immer abweisen (NACK)**: Telegramme werden aktiv mit NACK abgewiesen. **Achtung:** Bei dieser Einstellung ist eine Programmierung nur noch über IP möglich.
