#include <SPI.h>
#include <SD.h>

#define SD_CS            10      
#define LED_BLUE         9       
#define CHECK_INTERVAL   5000    // Reduced polling frequency (was 2000)

char dataFile[] = "cansat.txt";

bool sdReady = false;
unsigned long lastCheck = 0;
bool lastReportedState = false;

// Debounce counters
byte failCounter = 0;
byte successCounter = 0;

// Skip checks during write (prevent interference)
volatile bool isWriting = false;

void setup() {
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_BLUE, LOW); 
    Serial.begin(9600);
    
    Serial.println(F("--- NANO BOOT ---"));
    if (SD.begin(SD_CS)) {
        sdReady = true;
        lastReportedState = true;
        digitalWrite(LED_BLUE, HIGH);
        Serial.println(F("SD_CARD: OK"));
    } else {
        Serial.println(F("SD_CARD: ERR"));
    }
    Serial.println(F("NANO_READY"));
}

// Returns true if the SD card is physically present and accessible
bool isCardPresent() {
    // Attempt to initialise the card (forces fresh SPI communication)
    if (SD.begin(SD_CS)) {
        return true;
    }
    return false;
}

void checkSDCard() {
    if (isWriting) return;
    if (millis() - lastCheck < CHECK_INTERVAL) return;
    lastCheck = millis();
    
    bool present = isCardPresent();
    
    if (present) {
        if (!sdReady) {
            // Card was absent, now present – need two consecutive successes
            successCounter++;
            if (successCounter >= 2) {
                sdReady = true;
                lastReportedState = true;
                digitalWrite(LED_BLUE, HIGH);
                Serial.println("SD_READY");
                // Blink once to indicate reinsertion
                digitalWrite(LED_BLUE, LOW);
                delay(50);
                digitalWrite(LED_BLUE, HIGH);
                successCounter = 0;
                failCounter = 0;
            }
        } else {
            // Already ready – reset failure counter
            failCounter = 0;
        }
    } else {
        if (sdReady) {
            // Card was present, now missing – need three consecutive failures
            failCounter++;
            if (failCounter >= 3) {
                sdReady = false;
                lastReportedState = false;
                digitalWrite(LED_BLUE, LOW);
                Serial.println("SD_FAIL");
                failCounter = 0;
                successCounter = 0;
            }
        } else {
            // Already failed – reset success counter
            successCounter = 0;
        }
    }
}

void acknowledgeWrite(bool success) {
    if (success) {
        Serial.println("SD_WRITE_OK");
        if (!sdReady) {
            // A successful write implies the card is ready
            failCounter = 0;
            successCounter = 1;   // next check will confirm
        }
    } else {
        Serial.println("SD_WRITE_ERR");
        if (sdReady) {
            // Immediate failure on write error
            sdReady = false;
            lastReportedState = false;
            digitalWrite(LED_BLUE, LOW);
            Serial.println("SD_FAIL");
            failCounter = 0;
            successCounter = 0;
        }
    }
}

void printTableTo(Print &p, int lpSize, int pktNum, float t, float h, float pr, float al, float lt, float lg, float ax, float ay, float az) {
    p.println(F("\n+------------------------------------------------------------------------------+"));
    p.println(F("|                    GROUND STATION SAVE TELEMETRY TABLE                       |"));
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    p.println(F("| SENSOR               | PARAMETER                 | VALUE                     |"));
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    p.print(F("| DHT22                | Temperature               | ")); p.print(t); p.println(F("C                    |")); 
    p.print(F("| DHT22                | Humidity                  | ")); p.print(h); p.println(F("%                    |")); 
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    p.print(F("| BMP180               | Pressure                  | ")); p.print(pr, 0); p.println(F("Pa                   |")); 
    p.print(F("| BMP180               | Altitude (Est)            | ")); p.print(al); p.println(F("m                   |")); 
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    p.print(F("| MPU6050              | Accel X/Y/Z               | ")); 
    p.print(ax, 1); p.print(", "); p.print(ay, 1); p.print(", "); p.print(az, 1); p.println(F("          |")); 
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    if (lt != 0) { 
        p.print(F("| NEO-6M GPS           | Latitude                  | ")); p.println(lt, 6); 
        p.print(F("| NEO-6M GPS           | Longitude                 | ")); p.println(lg, 6); 
    } else { 
        p.println(F("| NEO-6M GPS           | Fix Status                | NO FIX / SEARCHING        |")); 
    } 
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    p.println(F("| CONNECTION INFO      | PARAMETER                 | VALUE                     |"));
    p.println(F("+----------------------+---------------------------+---------------------------+"));
    p.print(F("| DX-LR03-LoRa         | Last Packet Size          | ")); p.print(lpSize); p.println(F("Bytes                   |")); 
    p.print(F("| DX-LR03-LoRa         | Packet Number             | ")); p.print(pktNum); p.println(F("Bytes                   |")); 
    p.println(F("+----------------------+---------------------------+---------------------------+"));
}

void handleIncomingData(String payload) {
    int vals[2]; float f[10];
    int start = 0, comma;
    
    // Expecting: packet size (int), packet number (int), then 10 floats
    for(int i=0; i<12; i++) {
        comma = payload.indexOf(',', start);
        String s = (comma == -1) ? payload.substring(start) : payload.substring(start, comma);
        if(i < 2) vals[i] = s.toInt();
        else f[i-2] = s.toFloat();
        start = comma + 1;
    }

    printTableTo(Serial, vals[0], vals[1], f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]);

    isWriting = true;
    bool writeSuccess = false;
    
    if (sdReady) {
        File fTxt = SD.open(dataFile, FILE_WRITE);
        if (fTxt) {
            digitalWrite(LED_BLUE, LOW);
            fTxt.print("Time: "); fTxt.println(millis());
            printTableTo(fTxt, vals[0], vals[1], f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], f[8]);
            fTxt.println("---");
            fTxt.close();
            writeSuccess = true;
            digitalWrite(LED_BLUE, HIGH);
        } else {
            sdReady = false;
            digitalWrite(LED_BLUE, LOW);
        }
    }
    
    isWriting = false;
    acknowledgeWrite(writeSuccess);
}

void loop() {
    if (Serial.available()) {
        String req = Serial.readStringUntil('\n');
        req.trim();
        if (req == "BW16_HELLO" || req == "PING") {
            Serial.println(sdReady ? "SD_READY" : "SD_FAIL");
        }
        else if (req == "SD_STATUS") {
            // Immediate check (no debounce for manual query)
            bool present = isCardPresent();
            if (present != sdReady) {
                sdReady = present;
                lastReportedState = present;
                digitalWrite(LED_BLUE, present ? HIGH : LOW);
                failCounter = 0;
                successCounter = present ? 2 : 0;
            }
            Serial.println(sdReady ? "SD_READY" : "SD_FAIL");
        }
        else if (req.startsWith("PKT,")) {
            handleIncomingData(req.substring(4));
        }
    }
    checkSDCard();
}