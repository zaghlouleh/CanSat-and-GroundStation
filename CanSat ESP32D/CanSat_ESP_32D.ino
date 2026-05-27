#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h>

// --- PIN DEFINITIONS ---
#define DHTPIN 4
#define DHTTYPE DHT22
#define GPS_RX 16
#define GPS_TX 17
#define LORA_RX 26
#define LORA_TX 27

// --- OBJECTS ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial SerialGPS(2);
HardwareSerial SerialLoRa(1);

// --- VARIABLES ---
unsigned long lastDisplay = 0;
unsigned long lastStatus = 0;
const long interval = 2000;
String loraStatus = "INITIALIZING";
int lastPacketSize = 0;
int packetCounter = 0;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  SerialLoRa.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);
  
  Wire.begin();
  dht.begin();
  
  if (!bmp.begin()) {
    Serial.println("!!! BMP180 Not Found");
  }
  if (!mpu.begin()) {
    Serial.println("!!! MPU6050 Not Found");
  }
  
  loraStatus = "READY";
  packetCounter = 0;
  
  Serial.println("\nCANSAT MISSION READY - TRANSMITTER");
  Serial.println("Waiting for sensors to stabilize...");
  delay(2000);
}

void loop() {
  // Feed GPS parser (improved parsing)
  while (SerialGPS.available() > 0) {
    char c = SerialGPS.read();
    gps.encode(c);
    // Output raw NMEA sentences for debugging (optional)
    // Serial.print(c);
  }

  // Periodic status output
  if (millis() - lastStatus >= 10000) {
    lastStatus = millis();
    Serial.println("[STATUS] CanSat transmitter is operational");
  }

  if (millis() - lastDisplay >= interval) {
    lastDisplay = millis();
    sendTelemetry();
  }
}

void sendTelemetry() {
  // 1. GATHER DATA
  float dht_t = dht.readTemperature();
  float dht_h = dht.readHumidity();
  
  float bmp_t = bmp.readTemperature();
  float bmp_p = bmp.readPressure();
  float bmp_a = bmp.readAltitude();
  
  sensors_event_t a, g, mpu_temp;
  mpu.getEvent(&a, &g, &mpu_temp);

  // 2. PREPARE TELEMETRY STRING
  String loraMsg = "CS1,";
  loraMsg += String(dht_t, 1) + "," + String(dht_h, 1) + "," + String(bmp_p, 0) + ",";
  loraMsg += String(bmp_a, 1) + "," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + ",";
  loraMsg += String(a.acceleration.x, 1) + "," + String(a.acceleration.y, 1) + "," + String(a.acceleration.z, 1);
  
  // Add packet counter for debugging
  loraMsg += "," + String(packetCounter);
  
  // 3. BROADCAST VIA LORA
  size_t bytesSent = SerialLoRa.println(loraMsg);
  if (bytesSent > 0) {
    loraStatus = "ONLINE (TX OK)";
    lastPacketSize = bytesSent;
    packetCounter++;
  } else {
    loraStatus = "TX FAILED";
    lastPacketSize = 0;
  }

  // 4. PRINT TO SERIAL MONITOR (DEBUG)
    Serial.println("\n+------------------------------------------------------------------------------+");
  Serial.println("|                        CANSAT LIVE TELEMETRY TABLE                           |");
  Serial.println("+----------------------+---------------------------+---------------------------+");
  Serial.println("| SENSOR               | PARAMETER                 | VALUE                     |");
  Serial.println("+----------------------+---------------------------+---------------------------+");

  // DHT22 Section
  Serial.printf("| DHT22                | Humidity                  | %.2f %%                   |\n", dht_h);
  Serial.printf("| DHT22                | Temperature               | %.2f C                   |\n", dht_t);
  Serial.println("+----------------------+---------------------------+---------------------------+");
  // BMP180 Section
  Serial.printf("| BMP180               | Pressure                  | %.0f Pa                  |\n", bmp_p);
  Serial.printf("| BMP180               | Altitude (Est)            | %.2f m                  |\n", bmp_a);
  Serial.printf("| BMP180               | Temperature (Internal)    | %.2f C                   |\n", bmp_t);
  Serial.println("+----------------------+---------------------------+---------------------------+");
  // MPU6050 Section
  Serial.printf("| MPU6050              | Accel X / Y / Z           | %.1f, %.1f, %.1f           |\n", a.acceleration.x, a.acceleration.y, a.acceleration.z);
  Serial.printf("| MPU6050              | Gyro X / Y / Z            | %.1f, %.1f, %.1f           |\n", g.gyro.x, g.gyro.y, g.gyro.z);
  Serial.printf("| MPU6050              | Temperature (Die)         | %.2f C                   |\n", mpu_temp.temperature);
  Serial.println("+----------------------+---------------------------+---------------------------+");

  // GPS Section
  if (gps.location.isValid()) {
    Serial.printf("| NEO-6M GPS           | Latitude                  | %.6f                  |\n", gps.location.lat());
    Serial.printf("| NEO-6M GPS           | Longitude                 | %.6f                  |\n", gps.location.lng());
    Serial.printf("| NEO-6M GPS           | Sats in View              | %d                         |\n", gps.satellites.value());
    Serial.printf("| NEO-6M GPS           | Speed                     | %.2f kmh                |\n", gps.speed.kmph());
  } else {
    Serial.println("| NEO-6M GPS           | Fix Status                | NO FIX / SEARCHING        |");
  }
  Serial.println("+----------------------+---------------------------+---------------------------+");

  // LoRa Module Status Section (NEW)
  Serial.printf("| DX-LR03-LoRa         | Radio Status              | %s            |\n", loraStatus.c_str());
  Serial.printf("| DX-LR03-LoRa         | Last Packet Size          | %d Bytes                  |\n", lastPacketSize);
  Serial.printf("| DX-LR03-LoRa         | Frequency Band            | 433 MHz                   |\n");
  Serial.println("+----------------------+---------------------------+---------------------------+");
}