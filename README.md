# Windmesser – Adafruit Feather HUZZAH ESP8266

Firmware zur Messung der Windgeschwindigkeit über ein digitales Frequenzsignal
(0–700 Hz = 0–70 m/s), mit Web-Dashboard, 24h-Verlaufsgrafik, Maximalwert-Anzeige
und konfigurierbarem WLAN (Access Point oder Client).

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

- `/` – aktuelle Windgeschwindigkeit (m/s, km/h, kn, Beaufort), 24h-Grafik,
  Maximalwert mit Datum/Uhrzeit
- `/config` – WLAN-Konfiguration
- `/api/current` – JSON mit Momentanwerten
- `/api/history` – JSON-Array der 24h-Historie
- `/api/max` – JSON mit Maximalwert der letzten 24h

Die Oberfläche ist vollständig ohne externe Ressourcen (kein CDN) umgesetzt,
damit sie auch im Access-Point-Modus ohne Internetzugang funktioniert.

## Kalibrierung / Anpassung der Kennlinie

Die Kennlinie ist in `config.h` linear hinterlegt:

```cpp
#define FREQ_MIN_HZ    0.0f
#define FREQ_MAX_HZ    700.0f
#define SPEED_MIN_MS   0.0f
#define SPEED_MAX_MS   70.0f
```

Bei einem anderen Sensor einfach diese vier Werte anpassen.

## Datenhaltung / Einschränkungen

- Die 24h-Historie (1 Wert/Minute, 1440 Punkte) wird **im RAM** gehalten und
  geht bei einem Neustart/Stromausfall verloren. Für eine dauerhafte
  Speicherung könnte die Historie periodisch als Datei auf LittleFS
  geschrieben werden (im aktuellen Stand nicht umgesetzt, um Flash-
  Schreibzyklen zu schonen).
- Die Zeitsynchronisation erfolgt per NTP und benötigt daher einen
  Internetzugang (funktioniert also nur im STA-Modus mit funktionierendem
  Router-Internetzugang). Im reinen AP-Modus ohne Internet läuft die Uhr auf
  Basis der internen ESP-Zeit ab dem letzten erfolgreichen Sync bzw. bleibt
  bei „nicht synchronisiert“ stehen – die Historie wird dann erst nach einer
  erfolgreichen Zeitsynchronisation aufgezeichnet.

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

Der übrige Code (Sensorlogik, Historie, Web-Oberfläche) bleibt unverändert.
