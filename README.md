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
| **DHT11** | Raumtemperatur-Sensor | D8 |
| **LED** | Built-in LED | D13 |

## Pin-Konfiguration

```cpp
#define IR_SENSOR_OBJ A1    // Objekttemperatur (MLX90614 Ausgang)
#define IR_SENSOR_SUR A0    // Umgebungstemperatur (optional)
#define BUZZER_PIN 4        // Buzzer Piepton
#define BUTTON_PIN 3        // Starttaster
#define DHT_PIN 8           // DHT11 Raumtemperatur
```

## Installation

### Libraries installieren
1. Arduino IDE öffnen → Sketch → Bibliotheken einbinden → Bibliotheken verwalten
2. Installiere:
   - `Adafruit MLX90614 Library` (v2.1.6+)
   - `Grove - LCD RGB Backlight` (v1.0.2+)
   - `DHT sensor library` (v1.4.4+)
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

Die Temperaturmessung wird **automatisch kalibriert** mit dem DHT11-Sensor:

- **DHT11 misst Raumtemperatur** (echte Referenz)
- **IR-Sensor wird dynamisch angepasst** – Offset wird pro Messung berechnet
- **Keine manuelle Kalibrierung nötig!**

### Wie es funktioniert

1. DHT11 liest Raumtemperatur (z.B. 22°C)
2. IR-Sensor misst Rohwert (z.B. 0.498V → 37.4°C)
3. Offset = Raumtemp - IR-Rohwert (z.B. 22 - 37.4 = -15.4°C)
4. Kalibrierte Temp = IR-Rohwert + Offset (37.4 - 15.4 = 22°C)

### Beispiel

```
Raumtemp (DHT11): 22.0°C | Offset: -15.4°C
tempDirect=37.4°C, tempCalibrated=22.0°C ✓
```

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
DHT11 initialisiert
=== Setup abgeschlossen ===

=== Messung gestartet ===
Rohwert (gemittelt): 102 | Spannung OBJ: 0.498V | SUR: 0.107V
Raumtemp (DHT11): 22.0°C | Offset: -15.4°C
Formeln: tempMLX=-0.2°C, tempDirect=37.4°C, tempCalibrated=22.0°C
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
