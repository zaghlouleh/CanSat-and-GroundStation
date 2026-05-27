# CanSat Ground Station & Transmitter System

A complete **LoRa-based telemetry system** for CanSat projects, featuring a transmitter unit that collects sensor data and broadcasts via LoRa, a ground station receiver that displays data on an LCD, and an SD card logger for data persistence.

---

## 📋 Table of Contents

- [System Overview](#system-overview)
- [Key Features](#key-features)
- [Hardware Components](#hardware-components)
- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [Data Format](#data-format)
- [Configuration Guide](#configuration-guide)
- [Troubleshooting](#troubleshooting)
- [Documentation](#documentation)
- [License](#license)

---

## System Overview

The CanSat system is a **three-board telemetry chain** designed for high-altitude balloon experiments:

```
┌─────────────────────────────────────────────────────────────────┐
│  [ESP32D]──LoRa 433MHz──→[BW16]──UART 9600──→[Arduino Nano]     │
│  Transmitter   ~12 km     Receiver             Logger + SD Card │
│                         (LCD Display)                           │
│                                                                 │
│  • 6 Sensors         • LoRa RX           • Persistent Storage   │
│  • LoRa TX           • ST7920 LCD        • Data Timestamping    │
│  • 2s telemetry      • LED Indicators    • Human-readable       │
│                      • Bridge Logic      • CSV-compatible       │
└─────────────────────────────────────────────────────────────────┘
```

### Key Features

✅ **Multi-sensor telemetry** – DHT22, BMP180, MPU6050, NEO-6M GPS  
✅ **Long-range LoRa** – 433 MHz ISM band, ~12 km line-of-sight  
✅ **Real-time display** – ST7920 graphical LCD with live tables  
✅ **Persistent logging** – microSD with timestamped telemetry blocks  
✅ **Status indicators** – Non-blocking LED blinks (LoRa, SD, LCD)  
✅ **Modular firmware** – Each board independently programmable  
✅ **Open source** – MIT licensed, fully documented  

---

## Hardware Components

### Transmitter (ESP32D) – Flight Computer

| Component | Model | Purpose |
|-----------|-------|---------|
| **Microcontroller** | ESP32 | Main processor (240 MHz dual-core) |
| **Humidity/Temperature** | DHT22 | ±2°C, ±3% RH accuracy |
| **Pressure/Altitude** | BMP180 | Barometric measurement |
| **Accel/Gyro** | MPU6050 | 6-axis motion sensing |
| **GPS** | NEO-6M | GNSS positioning |
| **LoRa Radio** | DX-LR03 433 MHz | ~12 km transmission range |

**Example Sensor Data:**
```
Temperature:   25.7°C
Humidity:      37.8%
Pressure:      93099 Pa
Altitude:      707.93 m
Acceleration:  [-0.5, -0.4, 11.0] m/s²
GPS Status:    NO FIX (searching)
LoRa Status:   ONLINE (TX OK)
```

### Receiver (BW16 / RTL8720DN) – Ground Station Master

| Component | Model | Purpose |
|-----------|-------|---------|
| **Microcontroller** | RTL8720DN (BW16) | Main receiver & display controller |
| **LoRa Radio** | DX-LR03 433 MHz | Receives transmitter packets |
| **Graphical LCD** | ST7920 128×64 | Real-time telemetry visualization |
| **Status LEDs** | RGB + Discrete | System health indicators |
| **UART Bridge** | Serial interface | Communicates with Nano logger |

**LED Status Guide:**

| LED | Color | Meaning |
|-----|-------|---------|
| LoRa | 🟢 Green | Blinks on packet received |
| SD Card | 🔵 Blue | Solid when SD ready |
| Monitor | 🔴 Red | Blinks on LCD update |

### Logger (Arduino Nano) – Data Persistence

| Component | Model | Purpose |
|-----------|-------|---------|
| **Microcontroller** | ATmega328P (Nano) | Dedicated SD logger |
| **microSD Adapter** | Generic (AMS1117) | Non-volatile telemetry storage |
| **Status LED** | Blue | Indicates SD write activity |

**Output Format:**
```
Time: 14894ms

+────────────────────────────────────┐
| Temperature: 23.00°C               |
| Humidity:    40.60%                |
| Pressure:    93366 Pa              |
| Altitude:    684.70 m              |
| Accel XYZ:   -0.6, -0.5, 11.0      |
+────────────────────────────────────┘
---
```

---

## Quick Start

### 1. Prepare Hardware

**ESP32D Transmitter:**
- Assemble sensors on breadboard
- Connect DHT22, BMP180, MPU6050, NEO-6M, DX-LR03 to correct pins
- Power via USB or LiPo battery

**BW16 Receiver:**
- Connect LoRa module to Serial1 (PB1/PB2)
- Connect ST7920 LCD via SPI (PA25/PA27/PA26)
- Connect status LEDs (PA30/PA7/PA12)
- Connect Nano bridge (PA7/PA8)

**Arduino Nano Logger:**
- Connect microSD adapter via SPI (D10/D11/D12/D13)
- Connect status LED (D9)
- Connect UART from BW16 (D0/D1)

### 2. Install Libraries

In Arduino IDE → Sketch → Include Library → Manage Libraries, install:

```
DHT sensor library              (Adafruit)
Adafruit BMP085                 (Adafruit)
Adafruit MPU6050                (Adafruit)
Adafruit Unified Sensor         (Adafruit)
TinyGPS++                        (Mikal Hart)
U8g2                            (Oliver Krause)
SD                              (Arduino)
SPI                             (Arduino)
Wire                            (Arduino)
SoftwareSerial                  (Arduino)
```

### 3. Configure LoRa Modules

**Both master and slave must use identical settings:**

| Parameter | Value |
|-----------|-------|
| MAC Address | `00,01` |
| Channel | `03` |
| Spreading Factor | 12 (LEVEL 0) |
| Frequency | ~4372 MHz |

See [TUTORIALS.md](TUTORIALS.md#lora-module-configuration) for step-by-step AT command configuration.

### 4. Upload Firmware

**ESP32D:**
- Board: ESP32 Dev Module
- Upload `CanSat ESP32D/CanSat_ESP_32D.ino`
- Verify at 115200 baud: `CANSAT MISSION READY`

**BW16:**
- Board: BW16 (or Arduino Leonardo)
- Upload `Ground Station BW16/Ground_Station_BW16.ino`
- Verify at 115200 baud: `GROUND STATION BOOTING`

**Nano:**
- Board: Arduino Nano (ATmega328P)
- Upload `Ground Station NANO/Ground_Station_NANO.ino`
- Verify at 9600 baud: `NANO BOOT`, `SD_CARD: OK`

### 5. Monitor & Verify

Open three Serial Monitor windows:

```
┌─ Serial Monitor 115200 (ESP32D) ───┐
| CANSAT LIVE TELEMETRY TABLE        |
| ├─ DHT22:     25.7°C, 37.8%        |
| ├─ BMP180:    93099 Pa, 707.93 m   |
| ├─ MPU6050:   -0.5, -0.4, 11.0     |
| └─ LoRa:      ONLINE (TX OK)       |
└────────────────────────────────────┘

┌─ Serial Monitor 115200 (BW16) ─────┐
| GROUND STATION BOOTING             |
| PKT,60,0,23.0,40.6,93366...        |
| [STATION ALIVE] Total Packets: 1   |
└────────────────────────────────────┘

┌─ Serial Monitor 9600 (Nano) ───────┐
| SD_CARD: OK                        |
| NANO_READY                         |
| SD_WRITE_OK                        |
└────────────────────────────────────┘
```

If all three show data → **System is operational!** 🎉

---

## Architecture

### Data Flow Pipeline

```
1. TRANSMITTER (ESP32D)
   ├─→ Read sensors (DHT22, BMP180, MPU6050, NEO-6M)
   ├─→ Format LoRa frame: CS1,<data>...
   ├─→ Broadcast via Serial1 (DX-LR03)
   └─→ Print table to Serial Monitor (115200 baud)

2. RECEIVER (BW16)
   ├─→ Listen on Serial1 (LoRa module)
   ├─→ Validate: starts with "CS1,"
   ├─→ Parse 10 comma-separated fields
   ├─→ Compute tilt angle
   ├─→ Update ST7920 LCD (SPI)
   ├─→ Blink green LED (non-blocking)
   ├─→ Forward as PKT line to Nano (Serial PA7, 9600 baud)
   └─→ Print table to Serial Monitor (115200 baud)

3. LOGGER (Nano)
   ├─→ Receive PKT line (Serial D0, 9600 baud)
   ├─→ Parse fields
   ├─→ Print save table to Serial Monitor (9600 baud)
   ├─→ Append to cansat.txt on microSD
   ├─→ Send "SD_WRITE_OK" back to BW16 (Serial D1)
   └─→ Blink blue LED during write
```

### Wiring Diagram Summary

**ESP32D Pins:**
```
GPIO  4  → DHT22 Data (1-wire)
GPIO 21  → BMP180/MPU6050 SDA (I2C)
GPIO 22  → BMP180/MPU6050 SCL (I2C)
GPIO 16  → NEO-6M RX (Serial2)
GPIO 17  → NEO-6M TX (Serial2)
GPIO 26  → DX-LR03 RX (Serial1)
GPIO 27  → DX-LR03 TX (Serial1)
```

**BW16 Pins:**
```
PB2      → DX-LR03 RX (Serial1)
PB1      → DX-LR03 TX (Serial1)
PA25     → ST7920 E (SCLK)
PA27     → ST7920 RW (SID)
PA26     → ST7920 RS (CS)
PA30     → Green LED (LoRa)
PA7      → Blue LED (SD) / TX to Nano
PA8      → RX from Nano
PA12     → Red LED (Monitor)
PA13/PA14/PB3 → Internal RGB
```

**Nano Pins:**
```
D10      → SD CS
D11      → SD MOSI
D12      → SD MISO
D13      → SD SCK
D9       → Status LED
D0       → RX from BW16 (PA7)
D1       → TX to BW16 (PA8)
```

⚠️ **Voltage Warning:** Use a logic level shifter on Nano D1 → BW16 PA8 (5V to 3.3V conversion).

---

## Data Format

### LoRa Frame (ESP32D → BW16)

```
CS1,<t>,<h>,<p>,<a>,<lat>,<lng>,<ax>,<ay>,<az>,<pkt>

Field    Type    Example      Unit      Notes
─────────────────────────────────────────────────────
t        float   25.7         °C        Temperature
h        float   37.8         %         Humidity
p        float   93099.0      Pa        Pressure
a        float   707.93       m         Altitude
lat      float   0.000000     degrees   Latitude
lng      float   0.000000     degrees   Longitude
ax       float   -0.5         m/s²      Accel X
ay       float   -0.4         m/s²      Accel Y
az       float   11.0         m/s²      Accel Z
pkt      int     0            —         Packet counter

Example: CS1,25.7,37.8,93099,707.93,0.0,0.0,-0.5,-0.4,11.0,0
Size:    ~60 bytes
```

### Nano PKT Frame (BW16 → Nano)

```
PKT,<size>,<pkt>,<t>,<h>,<p>,<a>,<lat>,<lng>,<ax>,<ay>,<az>,<tilt>

Field    Type    Example      Unit      Notes
──────────────────────────────────────────────────────
size     int     60           bytes     Original LoRa line length
pkt      int     0            —         Packet number
t        float   23.0         °C        Temperature
h        float   40.6         %         Humidity
p        float   93366.0      Pa        Pressure
a        float   684.7        m         Altitude
lat      float   0.000000     degrees   Latitude
lng      float   0.000000     degrees   Longitude
ax       float   -0.6         m/s²      Accel X
ay       float   -0.5         m/s²      Accel Y
az       float   11.0         m/s²      Accel Z
tilt     float   4.06         degrees   Computed tilt

Example: PKT,60,0,23.0,40.6,93366,684.7,0.0,0.0,-0.6,-0.5,11.0,4.06
```

---

## Configuration Guide

### LoRa AT Commands

Both LoRa modules must use identical settings. See [TUTORIALS.md#lora-module-configuration](TUTORIALS.md#lora-module-configuration) for detailed steps.

**Quick Reference:**

```
+++                    # Enter AT mode
AT+HELP                # List all parameters
AT+MAC00,01            # Set MAC address
AT+CHANNEL03           # Set channel
AT+LEVEL0              # Set LEVEL (Spreading Factor)
+++                    # Exit AT mode
```

**Parameter Settings:**

| Parameter | Setting | Notes |
|-----------|---------|-------|
| MAC | `00,01` | Must match on both modules |
| Channel | `03` | Must match on both modules |
| LEVEL | `0` | 0 = SF12 (244 bps), 7 = SF5 (13020 bps) |
| Frequency | ~4372 MHz | 433 MHz ISM band |
| CRC | Disabled (0) | Must match |
| Power | 5 | Transmit power (0-7) |

### Serial Monitor Baud Rates

| Board | Baud Rate | Purpose |
|-------|-----------|---------|
| ESP32D | 115200 | Transmitter diagnostics |
| BW16 | 115200 | Receiver diagnostics |
| Nano | **9600** | SD logger diagnostics |

---

## Troubleshooting

### No LoRa Reception

**Problem:** BW16 doesn't receive packets from ESP32D

**Solution:**
1. Verify LoRa AT settings: `AT+HELP` on both modules
2. Ensure MAC, CHANNEL, and LEVEL match exactly
3. Check antenna connections
4. Test modules in close proximity (<1 meter)
5. Verify UART connections (GPIO 26/27 on ESP32D, PB1/PB2 on BW16)

### LCD Not Updating

**Problem:** BW16 LCD shows garbage or blank screen

**Solution:**
1. Verify SPI pins: PA25 (SCLK), PA27 (SID), PA26 (CS)
2. Ensure ST7920 PSB is connected to GND (critical!)
3. Check LCD voltage (5V for better contrast)
4. Test with U8g2 example sketches

### SD Card Not Writing

**Problem:** Nano detects SD card but writes fail

**Solution:**
1. Format SD card to FAT32
2. Verify SPI pins: D10 (CS), D11 (MOSI), D12 (MISO), D13 (SCK)
3. Check adapter power: VCC = 5V
4. Test with Arduino IDE SD examples
5. Check packet parsing logic

### LEDs Not Working

**Problem:** Status LEDs don't light up or stay on

**Solution:**
1. Verify GPIO pin assignments in firmware
2. Check LED polarity: anode to GPIO, cathode to GND
3. Verify current-limiting resistors (220–470Ω)
4. Test with simple `digitalWrite()` sketch

### BW16-Nano Communication Issues

**Problem:** BW16 sends packets but Nano doesn't receive

**Solution:**
1. Verify baud rates: BW16 uses 9600, Nano expects 9600
2. Check UART pins: PA7/PA8 (BW16) ↔ D0/D1 (Nano)
3. Add logic level shifter on Nano TX (5V → 3.3V)
4. Verify GND is common between boards

---

## Documentation

### In This Repository

| File | Purpose |
|------|---------|
| [TUTORIALS.md](TUTORIALS.md) | Step-by-step guides (LoRa config, uploads, debugging) |
| [Connection_Logic.md](Connection_Logic.md) | Detailed wiring tables and pinout |
| [SerialMonitor&LoRa Docs/](SerialMonitor&LoRa%20Docs/) | Serial output examples and LoRa AT sessions |
| [LICENSE](LICENSE) | MIT License |
| [REUSE.toml](REUSE.toml) | SPDX reuse compliance |

### Serial Monitor Output Examples

**ESP32D (Transmitter @ 115200 baud):**
```
CANSAT MISSION READY - TRANSMITTER

+───────────────────────────────────────────────────
| CANSAT LIVE TELEMETRY TABLE                      |
├─ DHT22:    Humidity 37.80%, Temp 25.70°C         |
├─ BMP180:   Pressure 93099 Pa, Alt 707.93 m       |
├─ MPU6050:  Accel -0.5, -0.4, 11.0 m/s²           |
├─ NEO-6M:   NO FIX / SEARCHING                    |
├─ LoRa:     ONLINE (TX OK), 62 Bytes, 433 MHz     |
└───────────────────────────────────────────────────

[STATUS] CanSat transmitter is operational
```

**BW16 (Receiver @ 115200 baud):**
```
GROUND STATION BOOTING (BW16)
[OK] LoRa UART (Serial1) Initialized.
BW16_HELLO
[SD] Status changed: READY

+───────────────────────────────────────────────────
| GROUND STATION LIVE TELEMETRY TABLE              |
├─ DHT22:    Temp 23.00°C, Humidity 40.60%         |
├─ BMP180:   Pressure 93366 Pa, Alt 684.70 m       |
├─ MPU6050:  Accel -0.6, -0.5, 11.0 m/s²           |
├─ LoRa:     Packet #0, 60 Bytes                   |
├─ Nano:     SD READY                              |
└───────────────────────────────────────────────────

PKT,60,0,23.00,40.60,93366.00,684.70,0.0,0.0,-0.60,-0.50,11.00,4.06
[STATION ALIVE] Total Packets: 1
```

**Nano (Logger @ 9600 baud):**
```
NANO BOOT
SD_CARD: OK
NANO_READY
SD_READY

+───────────────────────────────────────────────────
| GROUND STATION SAVE TELEMETRY TABLE              |
├─ DHT22:    Temp 23.00°C, Humidity 40.60%         |
├─ BMP180:   Pressure 93366 Pa, Alt 684.70 m       |
├─ MPU6050:  Accel -0.6, -0.5, 11.0 m/s²           |
└───────────────────────────────────────────────────

SD_WRITE_OK
SD_READY
```

---

## SD Card Data Analysis

### File Format (cansat.txt)

```
Time: 14894

+────────────────────────────────────────────┐
| GROUND STATION SAVE TELEMETRY TABLE        |
├─ Temperature:     23.00°C                  |
├─ Humidity:        40.60%                   |
├─ Pressure:        93366 Pa                 |
├─ Altitude:        684.70 m                 |
├─ Accel X/Y/Z:     -0.6, -0.5, 11.0         |
├─ Packet Size:     60 Bytes                 |
├─ Packet Number:   0                        |
└────────────────────────────────────────────┘
---
Time: 17073
...
```

### Recovery & Analysis

1. Remove microSD from Nano
2. Insert into PC's SD reader
3. Open `cansat.txt` in any text editor
4. Copy data to spreadsheet or parse with Python

**Python parsing example:**

```python
import re

with open('cansat.txt', 'r') as f:
    content = f.read()

blocks = content.split('---')
for block in blocks:
    time_match = re.search(r'Time:\s*(\d+)', block)
    temp_match = re.search(r'Temperature:\s*([\d.]+)', block)
    
    if time_match and temp_match:
        print(f"Time={time_match.group(1)}ms, Temp={temp_match.group(1)}°C")
```

---

## Contributing

We welcome contributions! Please:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/description`
3. Commit changes: `git commit -m "Clear description"`
4. Push to fork: `git push origin feature/description`
5. Submit a pull request

**Code style:**
- Use snake_case for variables and functions
- Add comments for non-obvious logic
- Keep functions under 50 lines
- Use meaningful variable names

---

## License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) for full details.

```
Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software...
```

---

## Support

**Questions or issues?**

1. Check [Troubleshooting](#troubleshooting) section
2. Review [TUTORIALS.md](TUTORIALS.md) for step-by-step guides
3. Inspect Serial Monitor output from all three boards
4. Check wiring against [Connection_Logic.md](Connection_Logic.md)
5. Review [SerialMonitor&LoRa Docs/](SerialMonitor&LoRa%20Docs/) for examples

---

**Last Updated:** May 27, 2026  
**Status:** Stable (v1.0)  
**Maintainers:** CanSat Development Team
