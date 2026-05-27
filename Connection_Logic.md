# Connection Logic (Pinout + Wiring) — CanSat Ground Station

This document is the improved, human-friendly version of the project’s wiring tables.

> Tip: Treat these as wiring/pinout guidance. Always cross-check with the exact board/sensor/module versions used.

---

## 1) Transmitter Unit (CanSat Payload) — ESP32

| Component | Module pin | ESP32 pin | Notes |
|---|---|---|---|
| DHT22 | Data | GPIO 4 | Requires **10k pull-up to 3.3V** |
| NEO-6M GPS | TX | GPIO 16 (**RX2**) | Serial2 input |
| NEO-6M GPS | RX | GPIO 17 (**TX2**) | Serial2 output |
| DX-LR03 LoRa | TX | GPIO 26 (**RX1**) | Serial1 input |
| DX-LR03 LoRa | RX | GPIO 27 (**TX1**) | Serial1 output |
| BMP180 | SDA | GPIO 21 | I2C bus |
| BMP180 | SCL | GPIO 22 | I2C bus |
| MPU6050 | SDA | GPIO 21 | Shared I2C bus |
| MPU6050 | SCL | GPIO 22 | Shared I2C bus |

---

## 2) Ground Station Main Controller (BW16) — RTL8720DN + ST7920

| Component | Module pin | BW16 pin | Notes |
|---|---|---|---|
| DX-LR03 LoRa | TX | PB2 (**RX1**) | Hardware Serial1 |
| DX-LR03 LoRa | RX | PB1 (**TX1**) | Hardware Serial1 |
| ST7920 LCD | E (SCLK) | PA25 | SPI (software SPI) |
| ST7920 LCD | RW (SID) | PA27 | SPI (software SPI) |
| ST7920 LCD | RS (CS) | PA26 | SPI (software SPI) |
| ST7920 LCD | PSB | GND | **Critical:** selects Serial mode |
| Status LED | Green (LoRa) | PA30 | LoRa activity indicator |
| Status LED | Blue (Nano) | PA7 | Nano bridge status |
| Status LED | Red (Monitor) | PA12 | LCD/system indicator |
| Internal RGB | R/G/B | PA13, PA14, PB3 | Internal status mirroring |

---

## 3) Ground Station Data Logger — Arduino Nano + microSD

| Component | Adapter pin | Nano pin | Notes |
|---|---|---|---|
| microSD Adapter | CS | D10 | Hardware SPI SS |
| microSD Adapter | MOSI | D11 | SPI |
| microSD Adapter | MISO | D12 | SPI |
| microSD Adapter | SCK | D13 | SPI |
| Local/status LED | Blue | D9 | Blinks during SD writes |

---

## 4) Inter-Board Communication — BW16 ↔ Nano (UART bridge)

| BW16 pin | Nano pin | Signal direction | Purpose |
|---|---|---|---|
| PA7 (**TX**) | D0 (**RX**) | BW16 → Nano | Telemetry packet forwarding |
| PA8 (**RX**) | D1 (**TX**) | Nano → BW16 | SD/handshake/status responses |
| GND | GND | Common ground | Required for signal integrity |

### Voltage safety (important)
- **BW16 is a 3.3V device**
- **Nano is a 5V device**

Use a **logic level shifter** (recommended) or a **voltage divider** on **Nano TX (D1)** before it enters **BW16 RX (PA8)**.

---

## 5) Power Distribution Logic (high-level)

1. **Transmitter / sensors / LoRa (ESP32 payload):**
   - Often powered by a LiPo (3.7–4.2V) and regulated to 3.3V as needed.

2. **Ground station (BW16):**
   - Powered via USB-C (commonly 5V).

3. **LCD (ST7920):**
   - Connect **VCC to 5V** (better contrast) and **GND to GND**.

4. **LoRa module (DX-LR03 on ground station):**
   - Connect **VCC** to **3.3V or 5V** depending on your module variant.
   - Note: DX-LR03 typically supports up to 5V VCC but uses **3.3V logic** on UART.

5. **Nano + microSD adapter:**
   - Nano can be powered from BW16’s 5V.
   - microSD adapters often include an onboard regulator (example mentioned: AMS1117) and expect adapter **VCC = 5V**.

---

## 6) Quick wiring checklist

- Verify UART wiring BW16↔Nano (PA7→D0, PA8→D1) and include **level shifting** on Nano TX.
- Verify LoRa UART wiring ESP32↔LoRa and BW16↔LoRa (TX→RX, RX→TX).
- Verify I2C wiring (BMP180 + MPU6050 both on GPIO 21/22).
- Verify ST7920 PSB pin is tied to **GND**.
- Verify microSD SPI wiring uses D10–D13.