# Fiebermesser mit Arduino Uno

Ein Arduino-basiertes Fiebermessgerät mit IR-Sensor, LCD-Display und Alarmsystem zur zuverlässigen Temperaturmessung und Fieberwarnung.

## Features

✅ **Berührungslose IR-Temperaturmessung** – MLX90614 analog Sensor  
✅ **LCD-Display** – Grove RGB LCD 16x2 mit Farbindikator (Grün=Normal, Rot=Fieber)  
✅ **Buzzer-Alarm** – 2x Piepton bei Fiebererkennung  
✅ **Button-Steuerung** – Messung auf Knopfdruck starten  
✅ **Stabile Messwerte** – Mehrfaches Sampling und Rauschfilter  
✅ **Auto-Kalibrierung** – Einfache Anpassung der Temperaturwerte  

## Hardware-Anforderungen

| Komponente | Beschreibung | Pin |
|-----------|-------------|-----|
| **Arduino Uno** | Microcontroller | — |
| **MLX90614 (analog)** | IR-Temperatursensor | A1 (Objekt), A0 (Umgebung) |
| **Grove RGB LCD** | 16x2 Display mit I2C | A4 (SDA), A5 (SCL), Addr: 0x3E |
| **Buzzer** | Piezo-Buzzer | D4 |
| **Button** | Taster (Push-Button) | D3 |
| **LED** | Built-in LED | D13 |

## Pin-Konfiguration

```cpp
#define IR_SENSOR_OBJ A1    // Objekttemperatur (MLX90614 Ausgang)
#define IR_SENSOR_SUR A0    // Umgebungstemperatur (optional)
#define BUZZER_PIN 4        // Buzzer Piepton
#define BUTTON_PIN 3        // Starttaster
```

## Installation

### Libraries installieren
1. Arduino IDE öffnen → Sketch → Bibliotheken einbinden → Bibliotheken verwalten
2. Installiere:
   - `Adafruit MLX90614 Library` (v2.1.6+)
   - `Grove - LCD RGB Backlight` (v1.0.2+)
   - `Wire` (Standard)

### Code hochladen
1. Sketch kompilieren: `Sketch → Verifizieren`
2. Board wählen: `Tools → Board → Arduino AVR Boards → Arduino Uno`
3. Port wählen: `Tools → Port → COM[X]`
4. Hochladen: `Sketch → Hochladen`

## Verwendung

1. **Button drücken** – Misst die Temperatur
2. **LCD zeigt an:**
   - Temperatur oben
   - Status unten (Normal/Fieber)
   - Farbe: **Grün = Normal** | **Rot = Fieber (>38.5°C)**
3. **Buzzer piept** – Bei Fieberwarnung 2x kurz
4. **Button loslassen** – Messung beendet

## Kalibrierung

Die Temperaturmessung wird durch zwei Parameter gesteuert:

```cpp
const float ANALOG_TEMP_GAIN = 75.0;     // mV/°C Faktor
const float ANALOG_TEMP_OFFSET = 0.0;    // °C Offset
```

### Kalibrierung durchführen

1. **Referenztemperatur messen** – Mit echtem Thermometer (z.B. 22°C Raumtemperatur)
2. **Sensor am Raum halten** – Nicht auf Haut
3. **Button drücken** – Serial Monitor öffnen (115200 Baud)
4. **Logs ansehen:**
   ```
   Spannung OBJ: 0.498V
   Formeln: tempDirect=37.4°C, tempCalibrated=37.4°C
   ```
5. **Berechnung:**
   - `ReferenzTemp = Spannung * GAIN + OFFSET`
   - Beispiel: `22 = 0.498 * GAIN + 0` → `GAIN = 44.2`

### Mehrpunkt-Kalibrierung

Für höhere Genauigkeit 2-3 Messwerte bei verschiedenen Temperaturen sammeln:

| Temperatur | Spannung | Berechnung |
|----------|----------|-----------|
| 20°C | 0.40V | Wert 1 |
| 37°C | 0.50V | Wert 2 |
| 40°C | 0.54V | Wert 3 |

Dann lineare Regression durchführen und GAIN/OFFSET anpassen.

## Troubleshooting

### Temperaturwerte unrealistisch
- **Lösung:** Raumtemperatur messen und Kalibrierung durchführen (siehe oben)
- Serial Monitor öffnen: `Tools → Serieller Monitor (115200 Baud)`

### LCD wird nicht erkannt
- I2C-Adresse prüfen (0x3E = RGB LCD)
- Verkabelung kontrollieren: SDA→A4, SCL→A5, VCC→5V, GND→GND

### Buzzer piept nicht
- Pin D4 überprüfen
- Buzzer-Polarität kontrollieren (+/−)
- Code: `digitalWrite(BUZZER_PIN, HIGH/LOW)`

### Sensor-Lesefehler
- Sensor ist auf analog eingestellt – keine I2C erforderlich
- Pins A0/A1 kontrollieren

## Serial Monitor Output

```
=== Setup start ===
Wire.begin() OK
Starte RGB LCD...
RGB LCD erkannt auf Adresse 0x3E
LCD initialisiert
Starte MLX90614 (analog)...
MLX90614 analog initialisiert!
=== Setup abgeschlossen ===

=== Messung gestartet ===
Rohwert (gemittelt): 102 | Spannung OBJ: 0.498V | SUR: 0.107V
Formeln: tempMLX=-0.2°C, tempDirect=37.4°C, tempCalibrated=37.4°C
Status: NORMAL
Messung beendet - Button loslassen
```

## RAM/Flash Speicher

- **Flash:** ~21 KB (65% von 32 KB)
- **RAM:** ~1.6 KB (80% von 2 KB) – Knapp, aber stabil

## Bekannte Einschränkungen

⚠️ OLED SSD1306 aktuell deaktiviert (I2C Konflikte mit LCD)  
⚠️ MLX90614 nur analog Mode (keine I2C)  
⚠️ RAM-Auslastung hoch – Wenig Platz für Erweiterungen  

## Autoren

Autor: Diego Trocker


Projekt entwickelt an der Oberschule Chr. J. Tschuggmall in Brixen (4.FI)


## Lizenz

MIT License – Frei verwendbar für Bildungszwecke
2026©
