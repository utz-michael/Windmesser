# Windmesser – Adafruit Feather HUZZAH ESP8266

Firmware zur Messung der Windgeschwindigkeit über ein digitales Frequenzsignal
(0–700 Hz = 0–70 m/s), mit Web-Dashboard für die aktuellen Live-Werte und
konfigurierbarem WLAN (Access Point oder Client).

## Hardware

- **Board:** Adafruit Feather HUZZAH ESP8266
- **Sensor-Eingang:** GPIO12 (D6), siehe `config.h` → `SENSOR_PIN`
- **Wichtig:** Der ESP8266 ist **nicht 5V-tolerant**. Liefert der Sensor ein
  5V-Rechtecksignal, muss ein Pegelwandler oder ein einfacher Spannungsteiler
  (z. B. 10 kΩ / 20 kΩ) zwischen Sensor und GPIO12 geschaltet werden.
- GND von Sensor und Feather müssen verbunden sein.

## Benötigte Arduino-Bibliotheken

Über den Arduino-Bibliotheksverwalter installieren:

1. **ESP8266-Boardpaket** (Board auswählen: *Adafruit Feather HUZZAH ESP8266*,
   Boardverwalter-URL: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`)
2. **ArduinoJson** (Version 6.x, von Benoit Blanchon)
3. **LittleFS** ist im ESP8266-Boardpaket bereits enthalten, keine separate
   Installation nötig.

## Projekt öffnen

1. Ordner `Windmesser/` in den Arduino-Sketch-Ordner legen (Ordnername und
   `.ino`-Dateiname müssen identisch sein – ist hier bereits der Fall).
2. `Windmesser.ino` in der Arduino-IDE öffnen. Die restlichen `.h`/`.cpp`-Dateien
   erscheinen automatisch als weitere Tabs.
3. Board auswählen: **Adafruit Feather HUZZAH ESP8266**, passenden Port wählen.
4. Hochladen.

## Erste Inbetriebnahme

Beim ersten Start (keine gespeicherte Konfiguration vorhanden) startet das
Gerät automatisch als **Access Point**:

- SSID: `Windmesser-Setup-XXXXXX` (XXXXXX = Chip-ID)
- Passwort: `windmesser`
- IP-Adresse: `192.168.4.1`

Mit einem Smartphone/Laptop verbinden und `http://192.168.4.1` im Browser
öffnen. Über den Link **„WLAN-Einstellungen“** kann auf **Client-Modus (STA)**
umgeschaltet werden: SSID und Passwort des Heim-WLANs eintragen, speichern.
Das Gerät startet neu und verbindet sich mit dem angegebenen Netzwerk. Bei
Fehlschlag (falsches Passwort, Netz nicht erreichbar) fällt das Gerät
automatisch wieder auf den Access-Point-Modus zurück.

Die aktuelle IP-Adresse im Client-Modus ist auf der seriellen Konsole
(115200 Baud) einsehbar.

## Web-Dashboard

- `/` – aktuelle Windgeschwindigkeit (m/s, km/h, kn, Beaufort), aktualisiert
  sich automatisch alle 5 Sekunden
- `/config` – WLAN-Konfiguration
- `/api/current` – JSON mit den Momentanwerten

Die Oberfläche ist vollständig ohne externe Ressourcen (kein CDN) umgesetzt,
damit sie auch im Access-Point-Modus ohne Internetzugang funktioniert.

## Speicher-Diagnose

Alle 60s wird auf der seriellen Konsole (115200 Baud) der freie Heap sowie
die Heap-Fragmentierung ausgegeben, z. B.:

```
[Heap] frei: 34120 Bytes, Fragmentierung: 8%
```

Ein kontinuierlich sinkender freier Heap deutet auf ein Speicherleck hin,
eine hohe/steigende Fragmentierung (>50%) erhöht das Risiko, dass eine
größere Allokation (z. B. für TCP-Puffer) fehlschlägt und das Gerät
abstürzt bzw. neu startet.

## Kalibrierung / Anpassung der Kennlinie

Die Kennlinie ist in `config.h` linear hinterlegt:

```cpp
#define FREQ_MIN_HZ    0.0f
#define FREQ_MAX_HZ    700.0f
#define SPEED_MIN_MS   0.0f
#define SPEED_MAX_MS   70.0f
```

Bei einem anderen Sensor einfach diese vier Werte anpassen.

## Beaufort-Skala (verwendete Grenzwerte in m/s)

| Stufe | bis (m/s) | Bezeichnung |
|---|---|---|
| 0 | 0,5 | Windstille |
| 1 | 1,6 | leiser Zug |
| 2 | 3,4 | leichte Brise |
| 3 | 5,5 | schwache Brise |
| 4 | 8,0 | mäßige Brise |
| 5 | 10,8 | frische Brise |
| 6 | 13,9 | starker Wind |
| 7 | 17,2 | steifer Wind |
| 8 | 20,8 | stürmischer Wind |
| 9 | 24,5 | Sturm |
| 10 | 28,5 | schwerer Sturm |
| 11 | 32,7 | orkanartiger Sturm |
| 12 | – | Orkan |

## Hinweis zur ESP32-Variante (Feather HUZZAH32)

Dieser Code ist für den **ESP8266**-basierten Feather HUZZAH geschrieben. Für
die ESP32-Variante (Adafruit Feather HUZZAH32) müssten folgende Header
ersetzt werden:

- `ESP8266WiFi.h` → `WiFi.h`
- `ESP8266WebServer.h` → `WebServer.h`
- `ESP.getChipId()` → z. B. `(uint32_t)ESP.getEfuseMac()`

Der übrige Code (Sensorlogik, Web-Oberfläche) bleibt unverändert.

## Frühere Funktionen (entfernt)

Frühere Versionen dieses Projekts enthielten eine 24h-Verlaufsgrafik sowie
eine Aufzeichnung/Anzeige des Maximalwerts der letzten 24h (inkl.
NTP-Zeitsynchronisation für Zeitstempel). Diese Funktionen wurden auf
Wunsch entfernt, um den Code schlank zu halten und den Speicherbedarf
weiter zu reduzieren. Bei Bedarf lässt sich das wieder ergänzen.
