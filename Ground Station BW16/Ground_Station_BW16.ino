// ============================================================
// CANSAT GROUND STATION – BW16 (RTL8720DN)
// LED LOGIC:
// - Red LED: ON continuous, blinks on every LCD update (non‑blocking)
// - Green LED: ON continuous, blinks on every LoRa packet received (non‑blocking)
// - Blue LED: SD card status (ON = ready, OFF = fail)
// ============================================================

#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <SoftwareSerial.h>

// ---- LED logic levels ----
#define LED_ON  HIGH
#define LED_OFF LOW

// ---- Pin definitions ----
#define LED_LORA_GREEN PA30   // Green LED (LoRa activity)
#define LED_SD_BLUE    PA7    // Blue LED (SD status)
#define LED_MON_RED    PA12   // Red LED (LCD monitor activity)

// Internal RGB (used as same functions)
#define RGB_RED        PA13
#define RGB_GREEN      PA14
#define RGB_BLUE       PB3

// LCD SPI
#define LCD_E          PA25
#define LCD_SID        PA27
#define LCD_CS         PA26

SoftwareSerial nanoSerial(PA8, PA7);  // RX=PA8, TX=PA7 to Arduino Nano
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, LCD_E, LCD_SID, LCD_CS, U8X8_PIN_NONE);

// Telemetry variables
float t_dht=0, hum=0, press=0, alt=0, lat=0, lng=0, ax=0, ay=0, az=0, tilt=0;
int pktNum=0, lastPktSize=0;      
unsigned long packetCount=0, lastHeartbeat=0, lastStatusRequest=0, lastPktSentTime=0;
bool nanoConnected = false, firstPacketReceived = false, waitingForAck = false;

// Debounce for LCD updates and status
unsigned long lastStatusChange = 0;
bool lastProcessedState = false;
unsigned long lastLcdUpdate = 0;

// Non‑blocking LED blink timers
unsigned long lastRedBlink = 0;
bool redBlinkActive = false;
unsigned long lastGreenBlink = 0;
bool greenBlinkActive = false;

// ------------------------------------------------------------------
// Non‑blocking LED control helpers
// ------------------------------------------------------------------
void setLcdLed(int state) {
    digitalWrite(LED_MON_RED, state);
    digitalWrite(RGB_RED, state);
}

void setLoRaLed(int state) {
    digitalWrite(LED_LORA_GREEN, state);
    digitalWrite(RGB_GREEN, state);
}

void setSdLed(int state) {
    digitalWrite(LED_SD_BLUE, state);
    digitalWrite(RGB_BLUE, state);
}

// Red blink: turns LED off for 30ms, then back on (non‑blocking)
void blinkRedAsync() {
    if (redBlinkActive) return;       // already blinking
    redBlinkActive = true;
    lastRedBlink = millis();
    setLcdLed(LED_OFF);
}

// Green blink: turns LED off for 30ms, then back on (non‑blocking)
void blinkGreenAsync() {
    if (greenBlinkActive) return;
    greenBlinkActive = true;
    lastGreenBlink = millis();
    setLoRaLed(LED_OFF);
}

void updateBlinkTimers() {
    unsigned long now = millis();
    if (redBlinkActive && (now - lastRedBlink >= 30)) {
        setLcdLed(LED_ON);
        redBlinkActive = false;
    }
    if (greenBlinkActive && (now - lastGreenBlink >= 30)) {
        setLoRaLed(LED_ON);
        greenBlinkActive = false;
    }
}

// ------------------------------------------------------------------
void requestSDStatus() {
    nanoSerial.println("SD_STATUS");
    lastStatusRequest = millis();
}

void sendPKTtoNano() {
    if (!nanoConnected) return;
    // Format: packetSize, packetNum, t, hum, press, alt, lat, lng, ax, ay, az, tilt
    nanoSerial.print("PKT,");
    nanoSerial.print(lastPktSize); nanoSerial.print(",");
    nanoSerial.print(pktNum); nanoSerial.print(",");
    nanoSerial.print(t_dht, 2); nanoSerial.print(",");
    nanoSerial.print(hum, 2); nanoSerial.print(",");
    nanoSerial.print(press, 2); nanoSerial.print(",");
    nanoSerial.print(alt, 2); nanoSerial.print(",");
    nanoSerial.print(lat, 6); nanoSerial.print(",");
    nanoSerial.print(lng, 6); nanoSerial.print(",");
    nanoSerial.print(ax, 2); nanoSerial.print(",");
    nanoSerial.print(ay, 2); nanoSerial.print(",");
    nanoSerial.print(az, 2); nanoSerial.print(",");
    nanoSerial.println(tilt, 2);
    waitingForAck = true;
    lastPktSentTime = millis();
}

// ------------------------------------------------------------------
void setup() {
    pinMode(LED_LORA_GREEN, OUTPUT);
    pinMode(LED_SD_BLUE, OUTPUT);
    pinMode(LED_MON_RED, OUTPUT);
    pinMode(RGB_RED, OUTPUT);
    pinMode(RGB_GREEN, OUTPUT);
    pinMode(RGB_BLUE, OUTPUT);

    // Start with all LEDs off
    setLcdLed(LED_OFF);
    setLoRaLed(LED_OFF);
    setSdLed(LED_OFF);

    Serial.begin(115200);
    nanoSerial.begin(9600);
    delay(2000);

    Serial.println("--- GROUND STATION BOOTING (BW16) ---");

    // Initialise LCD
    u8g2.begin();
    setLcdLed(LED_ON);      // Red LED ON (LCD active)
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(20, 35, "INIT SYSTEM...");
    u8g2.sendBuffer();
    blinkRedAsync();        // Non‑blocking blink after first draw

    // Initialise LoRa UART (Serial1)
    Serial1.begin(9600);
    Serial.println("[OK] LoRa UART (Serial1) Initialized.");
    setLoRaLed(LED_ON);     // Green LED ON (LoRa module ready)

    // Handshake with Arduino Nano
    nanoSerial.println("BW16_HELLO");
    delay(500);
    unsigned long startTime = millis();
    while (millis() - startTime < 2000) {
        if (nanoSerial.available()) {
            String response = nanoSerial.readStringUntil('\n');
            response.trim();
            if (response == "NANO_READY") {
                requestSDStatus();
                delay(100);
                if (nanoSerial.available()) {
                    String sdResp = nanoSerial.readStringUntil('\n');
                    sdResp.trim();
                    if (sdResp == "SD_READY") {
                        nanoConnected = true;
                        setSdLed(LED_ON);
                    } else {
                        nanoConnected = false;
                        setSdLed(LED_OFF);
                    }
                }
                break;
            }
        }
    }
    if (!nanoConnected) setSdLed(LED_OFF);
    displayWaitingScreen();
}

// ------------------------------------------------------------------
void displayWaitingScreen() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(15, 12, "GROUND STATION");
    u8g2.drawHLine(0, 15, 128);
    u8g2.drawStr(0, 35, "SD Card: ");
    u8g2.drawStr(65, 35, nanoConnected ? "VIA NANO" : "OFFLINE");
    u8g2.drawStr(0, 50, "Waiting for LoRa...");
    u8g2.sendBuffer();
    blinkRedAsync();
}

// ------------------------------------------------------------------
bool parseTelemetry(String data) {
    if (!data.startsWith("CS1,")) return false;
    lastPktSize = data.length();   // store for forwarding to Nano
    String payload = data.substring(4);
    int fieldIndex = 0, startPos = 0, commaPos = payload.indexOf(',');
    // Fields: t_dht, hum, press, alt, lat, lng, ax, ay, az, pktNum
    while (fieldIndex <= 10) {
        String field = (commaPos == -1) ? payload.substring(startPos) : payload.substring(startPos, commaPos);
        field.trim();
        switch (fieldIndex) {
            case 0: t_dht = field.toFloat(); break;
            case 1: hum   = field.toFloat(); break;
            case 2: press = field.toFloat(); break;
            case 3: alt   = field.toFloat(); break;
            case 4: lat   = field.toFloat(); break;
            case 5: lng   = field.toFloat(); break;
            case 6: ax    = field.toFloat(); break;
            case 7: ay    = field.toFloat(); break;
            case 8: az    = field.toFloat(); break;
            case 9: pktNum = field.toInt();  break;
        }
        if (commaPos == -1) break;
        startPos = commaPos + 1;
        commaPos = payload.indexOf(',', startPos);
        fieldIndex++;
    }

    float magnitude = sqrt(ax*ax + ay*ay + az*az);
    if (magnitude > 0.1) tilt = acos(az / magnitude) * 180.0 / PI;
    return true;
}

// ------------------------------------------------------------------
void updateLCD() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x7_tf);

    u8g2.drawStr(0, 7, "CANSAT LIVE DASHBOARD");
    u8g2.drawHLine(0, 9, 128);
    u8g2.setCursor(0, 18); u8g2.print("Pkt:"); u8g2.print(pktNum);
    u8g2.setCursor(0, 27); u8g2.print("Tmp:"); u8g2.print(t_dht, 1); u8g2.print("C");
    u8g2.setCursor(0, 36); u8g2.print("Hum:"); u8g2.print(hum, 0); u8g2.print("%");
    u8g2.setCursor(0, 45); u8g2.print("Prs:"); u8g2.print(press/100, 0); u8g2.print("hPa");
    u8g2.setCursor(64, 18); u8g2.print("Alt:"); u8g2.print(alt, 1); u8g2.print("m");
    u8g2.setCursor(64, 27); u8g2.print("Tlt:"); u8g2.print(tilt, 1); u8g2.print("deg");
    u8g2.setCursor(64, 36); u8g2.print("AccZ:"); u8g2.print(az, 1);
    u8g2.setCursor(64, 45); u8g2.print("SD :"); u8g2.print(nanoConnected ? "OK" : "ERR");
    u8g2.setCursor(0, 54); u8g2.print("A(X,Y): "); u8g2.print(ax, 1); u8g2.print(","); u8g2.print(ay, 1);
    u8g2.drawHLine(0, 56, 128);
    u8g2.setCursor(0, 63);
    if(lat != 0) {
        u8g2.print("GPS:"); u8g2.print(lat, 4); u8g2.print(","); u8g2.print(lng, 4);
    } else {
        u8g2.print("GPS: SEARCHING...");
    }
    u8g2.sendBuffer();
    blinkRedAsync();
}

// ------------------------------------------------------------------
void printTelemetryTable() {
    Serial.println("\n+------------------------------------------------------------------------------+");
    Serial.println("|                    GROUND STATION LIVE TELEMETRY TABLE                       |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.println("| SENSOR               | PARAMETER                 | VALUE                     |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.print("| DHT22                | Temperature               | "); Serial.print(t_dht); Serial.println(" C                   |");
    Serial.print("| DHT22                | Humidity                  | "); Serial.print(hum); Serial.println(" %                   |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.print("| BMP180               | Pressure                  | "); Serial.print(press, 0); Serial.println(" Pa                  |");
    Serial.print("| BMP180               | Altitude (Est)            | "); Serial.print(alt); Serial.println(" m                  |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.print("| MPU6050              | Accel X / Y / Z           | ");
    Serial.print(ax, 1); Serial.print(", "); Serial.print(ay, 1); Serial.print(", "); Serial.print(az, 1); Serial.println("          |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    if (lat != 0) {
        Serial.print("| NEO-6M GPS           | Latitude                  | "); Serial.println(lat, 6);
        Serial.print("| NEO-6M GPS           | Longitude                 | "); Serial.println(lng, 6);
    } else {
        Serial.println("| NEO-6M GPS           | Fix Status                | NO FIX / SEARCHING        |");
    }
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.println("| CONNECTION INFO      | PARAMETER                 | VALUE                     |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.print("| DX-LR03-LoRa         | Last Packet Size          | "); Serial.print(lastPktSize); Serial.println(" Bytes                  |");
    Serial.print("| DX-LR03-LoRa         | Packet Number             | "); Serial.print(pktNum); Serial.println("                         |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
    Serial.print("| Nano SD Logger       | Card Status               | "); Serial.print(nanoConnected ? "VIA NANO" : "OFFLINE"); Serial.println("                  |");
    Serial.println("+----------------------+---------------------------+---------------------------+");
}

// ------------------------------------------------------------------
void loop() {
    updateBlinkTimers();   // handle non‑blocking LED blinks

    // 1. Process LoRa packets
    if (Serial1.available()) {
        String received = Serial1.readStringUntil('\n');
        received.trim();
        if (received.length() > 0 && parseTelemetry(received)) {
            packetCount++;
            blinkGreenAsync();   // non‑blocking green blink

            if (!firstPacketReceived) {
                firstPacketReceived = true;
                updateLCD();
                lastLcdUpdate = millis();
            } else {
                // Throttle LCD updates to max 10 fps (100ms)
                if (millis() - lastLcdUpdate >= 100) {
                    updateLCD();
                    lastLcdUpdate = millis();
                }
            }
            printTelemetryTable();
            sendPKTtoNano();
        }
    }

    // 2. Process incoming messages from Nano (SD status updates)
    if (nanoSerial.available()) {
        String nanoMsg = nanoSerial.readStringUntil('\n');
        nanoMsg.trim();
        bool newState = nanoConnected;

        if (nanoMsg == "SD_WRITE_OK") {
            newState = true;
            waitingForAck = false;
        }
        else if (nanoMsg == "SD_WRITE_ERR") {
            newState = false;
            waitingForAck = false;
        }
        else if (nanoMsg == "SD_READY") {
            newState = true;
        }
        else if (nanoMsg == "SD_FAIL") {
            newState = false;
        }

        // Update only if state actually changed, debounce rapid toggles
        if (newState != lastProcessedState && (millis() - lastStatusChange > 500)) {
            lastProcessedState = newState;
            lastStatusChange = millis();
            nanoConnected = newState;
            setSdLed(newState ? LED_ON : LED_OFF);

            if (firstPacketReceived) updateLCD();
            else displayWaitingScreen();

            Serial.print("[SD] Status changed: ");
            Serial.println(newState ? "READY" : "FAIL");
        }
    }

    // 3. Timeout for missing ack
    if (waitingForAck && (millis() - lastPktSentTime > 300)) {
        waitingForAck = false;
        requestSDStatus();
    }

    // 4. Poll SD status every 5 seconds (reduced from 2s)
    if (millis() - lastStatusRequest > 5000) {
        requestSDStatus();
    }

    // 5. Redraw waiting screen periodically if no packet yet (every 2s)
    if (!firstPacketReceived && (millis() % 2000 < 50)) {
        displayWaitingScreen();
    }

    // 6. Health heartbeat (every 5 seconds)
    if (millis() - lastHeartbeat >= 5000) {
        lastHeartbeat = millis();
        Serial.print("[STATION ALIVE] Total Packets: "); Serial.println(packetCount);
        if (!nanoConnected && (millis() % 30000 < 1000)) {
            nanoSerial.println("PING");
            delay(50);
            if (nanoSerial.available()) {
                String resp = nanoSerial.readStringUntil('\n');
                if (resp == "PONG") requestSDStatus();
            }
        }
    }
}