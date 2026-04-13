#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "rgb_lcd.h"

// Arduino Uno Pin-Definitionen
#define IR_SENSOR_OBJ A1  // Objekttemperatur (analog)
#define IR_SENSOR_SUR A0  // Umgebungstemperatur (analog, optional)

// Zusätzliche Komponenten
#define BUZZER_PIN 4      // Buzzer an D4
#define BUTTON_PIN 3      // Button an D3

// 1. Pins für das Grove Shield (Hardware I2C)
// Die Hardware-I2C-Verbindung verwendet standardmäßig A4/A5
// Arduino Uno I2C: SDA = A4, SCL = A5
#define SDA_PIN A4
#define SCL_PIN A5

// 2. Sensor & LCD Instanzen
rgb_lcd lcd;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

const float FIEBER_GRENZE = 38.5; // Erhöht um falsche Alarme zu reduzieren

// Kalibrierung (anpassen, bis Werte realistisch sind):
//  - ANALOG_TEMP_GAIN: Passe an basierend auf Sensor. Typisch 100 für 10mV/°C, aber für IR-Sensor oft anders.
//  - ANALOG_TEMP_OFFSET: Passe an.
// Für deinen Sensor: Bei Raum ~35°C, Gain ~75, Offset ~0.
const float ANALOG_TEMP_GAIN = 75.0;
const float ANALOG_TEMP_OFFSET = 0.0;
const float ANALOG_TEMP_MIN = 10.0;
const float ANALOG_TEMP_MAX = 45.0;

bool sensorConnected = false;
bool lcdConnected = false;
bool isI2CSensor = false;

float readAnalogTemp() {
    // Mehrere Messungen für bessere Stabilität
    const int numSamples = 10;
    long sumObj = 0;
    long sumSur = 0;

    for(int i = 0; i < numSamples; i++) {
        sumObj += analogRead(IR_SENSOR_OBJ);
        sumSur += analogRead(IR_SENSOR_SUR);
        delay(10);
    }

    int rawObj = sumObj / numSamples;
    int rawSur = sumSur / numSamples;

    // Rauschfilter (Objektwert)
    static int lastRawObj = -1;
    if(lastRawObj == -1) {
        lastRawObj = rawObj;
    } else if(abs(rawObj - lastRawObj) > 50) {
        rawObj = lastRawObj;
        Serial.println("WARNUNG: Zu große Abweichung - verwende letzten Wert");
    } else {
        lastRawObj = rawObj;
    }

    float voltageObj = rawObj * 5.0 / 1024.0;
    float voltageSur = rawSur * 5.0 / 1024.0;

    // Messwert-Formeln (häufig MLX90614 analog-Clone)
    float tempMLXFormula = (voltageObj - 0.5) * 100.0; // typischer MLX90614 analog (deaktiviert, da falsch)
    float tempDirect = voltageObj * ANALOG_TEMP_GAIN;     // angepasste Formel
    float tempCalibrated = tempDirect + ANALOG_TEMP_OFFSET;

    Serial.print("Rohwert (gemittelt): ");
    Serial.print(rawObj);
    Serial.print(" | Spannung OBJ: ");
    Serial.print(voltageObj, 3);
    Serial.print("V | SUR: ");
    Serial.print(voltageSur, 3);
    Serial.println("V");

    Serial.print("Formeln: tempMLX=");
    Serial.print(tempMLXFormula, 1);
    Serial.print("°C, tempDirect=");
    Serial.print(tempDirect, 1);
    Serial.print("°C, tempCalibrated=");
    Serial.print(tempCalibrated, 1);
    Serial.println("°C");

    static float lastValidTemp = 25.0;
    float temp = tempCalibrated;

    if (temp < ANALOG_TEMP_MIN || temp > ANALOG_TEMP_MAX || isnan(temp)) {
        Serial.println("WARNUNG: kalibrierte Temperatur außerhalb Bereich, verwende letzten plausiblen Wert");
        temp = lastValidTemp;
    } else {
        lastValidTemp = temp;
    }

    return temp;
}

void setup() {
    Serial.begin(9600);
    delay(500);
    Serial.println("\n\n=== Setup start ===");
    Serial.flush();

    // I2C initialisieren
    Wire.begin();
    Serial.println("Wire.begin() OK");
    delay(200);
    Serial.flush();

    // LCD initialisieren (RGB LCD über I2C, Adresse 0x3E)
    Serial.println("Starte RGB LCD...");
    Serial.flush();
    delay(100);

    // Versuche, mit dem LCD zu kommunizieren - mit Timeout
    unsigned long lcdStart = millis();
    while(millis() - lcdStart < 1000) {
        Wire.beginTransmission(0x3E);  // RGB LCD Adresse
        if(Wire.endTransmission() == 0) {
            Serial.println("RGB LCD erkannt auf Adresse 0x3E");
            lcdConnected = true;
            break;
        }
        delay(50);
    }

    if(lcdConnected) {
        lcd.begin(16, 2);
        Serial.println("LCD initialisiert");
        lcd.setRGB(255, 255, 255);
        lcd.print("System...");
    } else {
        Serial.println("WARNUNG: RGB LCD nicht erkannt");
    }
    Serial.flush();
    delay(200);

    // MLX90614 Sensor initialisieren (analog)
    Serial.println("Starte MLX90614 (analog)...");
    Serial.flush();
    pinMode(IR_SENSOR_OBJ, INPUT);  // Stelle sicher, dass Pin als Input konfiguriert ist
    pinMode(IR_SENSOR_SUR, INPUT);
    Serial.println("MLX90614 analog initialisiert!");
    sensorConnected = true;
    isI2CSensor = false;
    Serial.flush();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Buzzer und Button initialisieren
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);  // Buzzer aus
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button mit internem Pullup

    Serial.println("=== Setup abgeschlossen ===");
    Serial.flush();
    delay(500);
}

void loop() {
    // blink the built‑in LED so we can see the sketch is running
    digitalWrite(LED_BUILTIN, millis() / 500 % 2);

    // Button lesen (LOW = gedrückt)
    bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

    if(buttonPressed) {
        Serial.println("=== Messung gestartet ===");

        // Temperatur lesen
        float temp = 0.0; // Variable außerhalb des if-Blocks deklarieren
        if(sensorConnected) {
            if(isI2CSensor) {
                temp = mlx.readObjectTempC();
                Serial.print("I2C Temperatur: ");
                Serial.print(temp, 1);
                Serial.println("°C");

                // Bei ungültigen I2C-Werten auf analog umschalten
                if(isnan(temp) || temp < 5.0 || temp > 60.0) {
                    Serial.println("WARNUNG: I2C Temperatur ungültig, fallback auf analog.");
                    isI2CSensor = false;
                    temp = readAnalogTemp();
                }
            } else {
                temp = readAnalogTemp();
            }

            // Erweiterte Fehlerkorrektur - realistische Körpertemperaturbereiche
            if (temp > ANALOG_TEMP_MAX || temp < ANALOG_TEMP_MIN || isnan(temp)) {
                Serial.print("WARNUNG: Temperatur ");
                Serial.print(temp, 1);
                Serial.println("°C außerhalb Bereich " + String(ANALOG_TEMP_MIN) + "-" + String(ANALOG_TEMP_MAX) + "°C oder NaN");

                if(isI2CSensor) {
                    temp = 25.0;  // Fallback für I2C
                    Serial.println("Auf 25.0°C gesetzt (I2C Fallback)");
                } else {
                    // Bei analoger Messung: Wert bleibt stabiler (letzter gültiger Wert wird in readAnalogTemp übernommen)
                    temp = readAnalogTemp();
                    if (temp < ANALOG_TEMP_MIN || temp > ANALOG_TEMP_MAX || isnan(temp)) {
                        temp = 25.0;
                        Serial.println("Auf 25.0°C gesetzt (Fallback)");
                    }
                }
            }
        } else {
            temp = 25.0;
            Serial.println("Warnung: Sensor nicht verbunden, setze 25°C");
        }

        // LCD Anzeige & Logik
        if(lcdConnected) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Temp: ");
            lcd.print(temp, 1);
            lcd.print(" C");

            lcd.setCursor(0, 1);
            if (temp > FIEBER_GRENZE) {
                lcd.setRGB(255, 0, 0); // Rot
                lcd.print("FIEBER!");
                Serial.println("Status: FIEBER ERKANNT!");

                // Buzzer-Alarm: 2x kurz piepen
                for(int i = 0; i < 2; i++) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    delay(100);  // Kurz piepen
                    digitalWrite(BUZZER_PIN, LOW);
                    if(i < 1) delay(100);  // Pause zwischen Pieptönen
                }
            } else {
                lcd.setRGB(0, 255, 0); // Grün
                lcd.print("Normal");
                Serial.println("Status: NORMAL");
                digitalWrite(BUZZER_PIN, LOW);

                // Kurzer Bestätigungston
                digitalWrite(BUZZER_PIN, HIGH);
                delay(50);
                digitalWrite(BUZZER_PIN, LOW);
            }
        }

        // Warten bis Button losgelassen wird
        while(digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }
        Serial.println("Messung beendet - Button loslassen\n");

    } else {
        // Keine Messung aktiv - Standby-Modus
        if(lcdConnected) {
            lcd.clear();
            lcd.setRGB(255, 255, 255);
            lcd.print("Druecken zum");
            lcd.setCursor(0, 1);
            lcd.print("Messen");
        }

        digitalWrite(BUZZER_PIN, LOW);
    }

    delay(100);  // Kurze Verzögerung für Button-Entprellung
}
