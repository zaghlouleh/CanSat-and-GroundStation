# CanSat System Tutorials & Guides

Complete step-by-step guides for setting up, configuring, and troubleshooting your CanSat telemetry system.

---

## 📚 Table of Contents

1. [LoRa Module Configuration](#lora-module-configuration)
2. [Serial Monitor Debugging](#serial-monitor-debugging)
3. [SD Card Data Analysis](#sd-card-data-analysis)
4. [Hardware Assembly & Wiring](#hardware-assembly--wiring)
5. [Firmware Upload Guide](#firmware-upload-guide)

---

## LoRa Module Configuration

Canonical reference (authoritative parameter set):
- `SerialMonitor&LoRa Docs/lora-configuration.md`
- `SerialMonitor&LoRa Docs/lora-master-at-session.md`
- `SerialMonitor&LoRa Docs/lora-slave-at-session.md`

TUTORIALS.md documents the practical workflow + troubleshooting; keep LoRa settings identical on both ends (ESP32D ↔ BW16) using the reference above.


---

### Quick Configuration Checklist

| Parameter | Master (ESP32D) | Slave (BW16) | Notes |
|-----------|-----------------|--------------|-------|
| MAC Address | `00,01` | `00,01` | **Must match** |
| Channel | `03` | `03` | **Must match** |
| LEVEL | `0` | `0` | **Must match** |
| Frequency | ~4372 MHz | ~4372 MHz | Should result from channel setting |
| Spreading Factor | 12 | 12 | Results from LEVEL=0 |
| Coding Rate | 2 | 2 | Standard LoRa |
| CRC | Disabled (0) | Disabled (0) | false/0 means disabled |
| Preamble | 8 | 8 | Standard length |
| Power | 5 | 5 | Transmit power |

---

### Step-by-Step Master (ESP32D) Configuration

#### Prerequisites

- Arduino IDE with ESP32 board support installed
- USB-to-UART cable connected to ESP32D's serial pins
- LoRa module powered and connected to Serial1 (GPIO 26/27)

#### Configuration Steps

1. **Open Serial Monitor**
   - Arduino IDE → Tools → Serial Monitor
   - Set baud rate to **115200**
   - Set line ending to **"No line ending"** (important!)

2. **Enter AT Mode**
   - Type: `+++` (three plus signs, no spaces or line ending)
   - Press: **Send** button (or Enter after setting "No line ending")
   - Module should respond with a prompt

3. **Check Current Configuration**
   - Type: `AT+HELP`
   - Press: **Send**
   - Module prints all current LoRa parameters

4. **Configure MAC Address**
   - Type: `AT+MAC00,01`
   - Press: **Send**
   - Expected response: `+MAC=00,01` then `OK`

5. **Configure Channel**
   - Type: `AT+CHANNEL03`
   - Press: **Send**
   - Expected response: `+CHANNEL=03` then `OK`

6. **Configure Spreading Factor (LEVEL)**
   - For longer range (slower data): `AT+LEVEL0`
   - For shorter range (faster data): `AT+LEVEL7`
   - Type: `AT+LEVEL0`
   - Press: **Send**
   - Expected response: `OK`

7. **Verify Configuration**
   - Type: `AT+HELP` again
   - Verify MAC, CHANNEL, Frequency, and Spreading Factor

8. **Exit AT Mode**
   - Type: `+++`
   - Press: **Send**
   - Module exits AT mode and resumes normal operation

#### Example Master Configuration Session

```
>>> +++
(AT mode prompt)

>>> AT+HELP
LoRa Parameter:
MODE:0
LEVEL:0 >> 244.140625bps
SLEEP:2
Frequency:4372
MAC:00,01
Spreading Factor:12
Coding rate:2
CRC:0(false)
Preamble:8
IQ:0(false)
Power:5

>>> AT+MAC00,01
+MAC=00,01
OK

>>> AT+CHANNEL03
+CHANNEL=03
OK

>>> AT+LEVEL0
OK

>>> AT+HELP
LoRa Parameter:
MODE:0
LEVEL:0 >> 244.140625bps
SLEEP:2
Frequency:4372
MAC:00,01
Spreading Factor:12
Coding rate:2
CRC:0(false)
Preamble:8
IQ:0(false)
Power:5

>>> +++
(exits AT mode)
```

---

### Step-by-Step Slave (BW16) Configuration

**Repeat the exact same steps as the Master, ensuring:**

1. **MAC, CHANNEL, and LEVEL match the Master exactly**
2. Serial Monitor is set to 115200 baud
3. LoRa module is connected to Serial1 (PB1/PB2 on BW16)

After configuration, both modules should show identical MAC, CHANNEL, LEVEL, and FREQUENCY values in `AT+HELP` output.

---

### LEVEL Settings & Data Rates

The LEVEL parameter controls the spreading factor and thus the range/speed trade-off:

| LEVEL | Spreading Factor | Bitrate | Range | Use Case |
|-------|------------------|---------|-------|----------|
| **0** | 12 | 244 bps | ~12 km | **Default (longest range)** |
| 1 | 11 | 434 bps | ~10 km | Medium range |
| 2 | 10 | 781 bps | ~8 km | Faster data |
| 3 | 9 | 1465 bps | ~6 km | Even faster |
| 4 | 8 | 2734 bps | ~4 km | High speed |
| 5 | 7 | 5468 bps | ~2 km | Very high speed |
| 6 | 6 | 10937 bps | ~1 km | Near-field only |
| **7** | 5 | 13020 bps | ~500 m | **Fastest (short range)** |

**Recommendation:** Use **LEVEL 0** for maximum range and reliability with stationary ground station. Use **LEVEL 7** if range is <500m and you need faster sensor readings.

---

### Troubleshooting LoRa Configuration

| Issue | Cause | Solution |
|-------|-------|----------|
| **Module won't enter AT mode** | Baud rate wrong or connection loose | Verify 115200 baud, check UART connections |
| **`AT+HELP` shows different MAC/CHANNEL** | Module not configured | Run configuration steps again |
| **No packets received after config** | LEVEL/MAC/CHANNEL mismatch between master/slave | Verify both modules have identical values via `AT+HELP` |
| **Packets received but corrupted** | Spreading factor or frequency mismatch | Ensure SF and frequency match on both modules |
| **Very short range** | Power setting too low | Increase with `AT+POWER7` (max) |
| **CRC errors in logs** | CRC enabled when it shouldn't be | Ensure `AT+HELP` shows `CRC:0(false)` |

---

## Serial Monitor Debugging

### Output Overview by Board

Each board prints different information to Serial Monitor at different baud rates:

| Board | Baud Rate | Output | Frequency |
|-------|-----------|--------|-----------|
| **ESP32D** | 115200 | Sensor readings + LoRa TX status | Every 2 seconds |
| **BW16** | 115200 | Parsed telemetry + forwarding info | On packet RX |
| **Nano** | 9600 | SD write status + saved data | On packet RX |

---

### ESP32D Transmitter Output

**What you should see:**

```
--- CANSAT MISSION READY ---
Waiting for sensors to stabilize...

+------------------------------------------------------------------------------+
|                        CANSAT LIVE TELEMETRY TABLE                           |
+----------------------+---------------------------+---------------------------+
| DHT22                | Humidity                  | 37.80 %                   |
| DHT22                | Temperature               | 25.70 C                   |
+----------------------+---------------------------+---------------------------+
| BMP180               | Pressure                  | 93099 Pa                  |
| BMP180               | Altitude (Est)            | 707.93 m                  |
| BMP180               | Temperature (Internal)    | 28.20 C                   |
+----------------------+---------------------------+---------------------------+
| MPU6050              | Accel X / Y / Z           | -0.5, -0.4, 11.0          |
| MPU6050              | Gyro X / Y / Z            | -0.1, 0.0, -0.0           |
| MPU6050              | Temperature (Die)         | 26.13 C                   |
+----------------------+---------------------------+---------------------------+
| NEO-6M GPS           | Fix Status                | NO FIX / SEARCHING        |
+----------------------+---------------------------+---------------------------+
| DX-LR03-LoRa         | Radio Status              | ONLINE (TX OK)            |
| DX-LR03-LoRa         | Last Packet Size          | 62 Bytes                  |
| DX-LR03-LoRa         | Frequency Band            | 433 MHz                   |
+----------------------+---------------------------+---------------------------+

[STATUS] CanSat transmitter is operational
```

**What each line means:**

- **DHT22 rows:** Current humidity and temperature from the DHT22 sensor
- **BMP180 rows:** Atmospheric pressure, altitude estimate, and internal sensor temperature
- **MPU6050 rows:** Acceleration vector (X, Y, Z) and rotation rates (gyroscope)
- **NEO-6M GPS row:** GPS fix status (`NO FIX` = still searching for satellites)
- **DX-LR03 rows:** LoRa radio status and last transmitted packet size
- **[STATUS] line:** Heartbeat message (appears every 10 seconds)

---

### BW16 Ground Station Receiver Output

**Boot sequence (what you should see on startup):**

```
--- GROUND STATION BOOTING (BW16) ---
[OK] LoRa UART (Serial1) Initialized.
BW16_HELLO
SD_STATUS
[STATION ALIVE] Total Packets: 0
SD_STATUS
[SD] Status changed: READY
```

**Meaning:**
- `[OK] LoRa UART...` → LoRa module is responding
- `BW16_HELLO` → BW16 sends greeting to Nano
- `SD_STATUS` → BW16 polls Nano for SD card status
- `[SD] Status changed: READY` → Nano reports SD card is available

**Live packet reception (repeats for each LoRa packet):**

```
+------------------------------------------------------------------------------+
|                    GROUND STATION LIVE TELEMETRY TABLE                       |
+----------------------+---------------------------+---------------------------+
| DHT22                | Temperature               | 23.00 C                   |
| DHT22                | Humidity                  | 40.60 %                   |
...
| Nano SD Logger       | Card Status               | VIA NANO                  |
+----------------------+---------------------------+---------------------------+

PKT,60,0,23.00,40.60,93366.00,684.70,0.000000,0.000000,-0.60,-0.50,11.00,4.06
[STATION ALIVE] Total Packets: 1
```

**Meaning:**
- Telemetry table: Parsed sensor values from the received LoRa packet
- `PKT,60,0,...`: Raw forwarding command to Nano (packet size, packet number, sensor data)
- `[STATION ALIVE]`: Heartbeat message with total packet count

---

### Arduino Nano Logger Output

**Boot sequence (9600 baud):**

```
--- NANO BOOT ---
SD_CARD: OK
NANO_READY
SD_READY
```

**Meaning:**
- `SD_CARD: OK` → SD card detected and initialized
- `NANO_READY` → Nano firmware is running
- `SD_READY` → SD card is ready for writes

**Packet reception (repeats for each PKT command from BW16):**

```
+------------------------------------------------------------------------------+
|                    GROUND STATION SAVE TELEMETRY TABLE                       |
+----------------------+---------------------------+---------------------------+
| DHT22                | Temperature               | 23.00C                    |
| DHT22                | Humidity                  | 40.60%                    |
...
+----------------------+---------------------------+---------------------------+

SD_WRITE_OK
SD_READY
```

**Meaning:**
- Telemetry table: Same data received from BW16
- `SD_WRITE_OK` → Data successfully appended to `cansat.txt`
- `SD_READY` → SD card still available

---

### Debug Checklist

✅ **All three Serial Monitors showing data?**
  1. ESP32D prints telemetry table every 2 seconds
  2. BW16 prints received packets
  3. Nano prints saved packets and `SD_WRITE_OK`
  
✅ **BW16 not receiving from ESP32D?**
  1. Verify LoRa module MAC/CHANNEL/LEVEL match
  2. Check UART connections (GPIO 26/27 on ESP32D → Serial1)
  3. Look for LoRa errors in BW16 Serial Monitor
  
✅ **Nano not receiving from BW16?**
  1. Verify BW16 can reach Nano (check BW16 Serial Monitor for `PKT,` lines)
  2. Check UART connections (PA7/PA8 on BW16 → D0/D1 on Nano)
  3. Verify baud rate is 9600 on Nano (not 115200)

---

## SD Card Data Analysis

### Reading the Data

#### On the Nano's microSD card:

The file `cansat.txt` contains timestamped blocks of telemetry data:

```
Time: 14894

+──────────────────────────────────────────────────
| GROUND STATION SAVE TELEMETRY TABLE              |
+──────────────────────────────────────────────────
| DHT22     | Temperature   | 23.00C              |
| DHT22     | Humidity      | 40.60%              |
| BMP180    | Pressure      | 93366Pa             |
| BMP180    | Altitude      | 684.70m             |
| MPU6050   | Accel X/Y/Z   | -0.6, -0.5, 11.0   |
| NEO-6M    | Fix Status    | NO FIX / SEARCHING  |
| LoRa      | Packet Size   | 60Bytes             |
| LoRa      | Packet Number | 0                   |
+──────────────────────────────────────────────────

---
Time: 17073

...
```

#### To recover the data:

1. **Remove microSD card from Nano**
2. **Insert into PC's SD card reader**
3. **Copy `cansat.txt` to computer**
4. **Open in any text editor** (Notepad, VS Code, etc.)

---

### Parsing Data Programmatically

#### Python Script Example

```python
import re

def parse_cansat_log(filename):
    """Parse cansat.txt and extract telemetry records."""
    
    with open(filename, 'r') as f:
        content = f.read()
    
    # Split by separator
    blocks = content.split('---')
    records = []
    
    for block in blocks:
        if not block.strip():
            continue
        
        record = {}
        
        # Extract timestamp
        time_match = re.search(r'Time:\s*(\d+)', block)
        if time_match:
            record['time_ms'] = int(time_match.group(1))
        
        # Extract temperature
        temp_match = re.search(r'Temperature\s*\|\s*([\d.]+)', block)
        if temp_match:
            record['temp_c'] = float(temp_match.group(1))
        
        # Extract humidity
        hum_match = re.search(r'Humidity\s*\|\s*([\d.]+)', block)
        if hum_match:
            record['humidity_%'] = float(hum_match.group(1))
        
        # Extract pressure
        press_match = re.search(r'Pressure\s*\|\s*([\d.]+)', block)
        if press_match:
            record['pressure_pa'] = float(press_match.group(1))
        
        # Extract altitude
        alt_match = re.search(r'Altitude\s*\|\s*([\d.]+)', block)
        if alt_match:
            record['altitude_m'] = float(alt_match.group(1))
        
        # Extract accel
        accel_match = re.search(r'Accel\s*\|\s*([\d\-., ]+)', block)
        if accel_match:
            accel_str = accel_match.group(1)
            accel_vals = [float(x.strip()) for x in accel_str.split(',')]
            record['accel_x'], record['accel_y'], record['accel_z'] = accel_vals
        
        if record:
            records.append(record)
    
    return records

# Usage
if __name__ == "__main__":
    records = parse_cansat_log('cansat.txt')
    
    for i, rec in enumerate(records):
        print(f"Record {i}: Time={rec.get('time_ms')}ms, "
              f"Temp={rec.get('temp_c')}°C, "
              f"Humidity={rec.get('humidity_%')}%, "
              f"Pressure={rec.get('pressure_pa')}Pa")
```

---

### Creating a Spreadsheet

1. **Copy data from `cansat.txt`** into a text file
2. **Open Excel or LibreOffice Calc**
3. **Data → Text to Columns** (if needed)
4. **Create columns:** Time, Temperature, Humidity, Pressure, Altitude, Accel_X, Accel_Y, Accel_Z
5. **Plot charts** using the time-series data

---

## Hardware Assembly & Wiring

### Transmitter (ESP32D) Assembly

#### Step 1: Identify Components

- ESP32 development board
- DHT22 sensor
- BMP180 barometric sensor
- MPU6050 accelerometer/gyroscope
- NEO-6M GPS module
- DX-LR03 LoRa module (433 MHz)
- Breadboard and jumper wires
- 10kΩ resistor for DHT22 pull-up

#### Step 2: Connect DHT22 (Temperature/Humidity)

| DHT22 Pin | ESP32 Pin |
|-----------|-----------|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO 4 (with 10kΩ pull-up to 3.3V) |

**Pull-up resistor:** Connect 10kΩ resistor between DHT22 DATA pin and 3.3V rail.

#### Step 3: Connect BMP180 & MPU6050 (I2C Bus)

Both sensors share the I2C bus (GPIO 21/22):

| Sensor | Pin | ESP32 Pin |
|--------|-----|-----------|
| BMP180 / MPU6050 | VCC | 3.3V |
| BMP180 / MPU6050 | GND | GND |
| BMP180 / MPU6050 | SDA | GPIO 21 |
| BMP180 / MPU6050 | SCL | GPIO 22 |

#### Step 4: Connect NEO-6M GPS (Serial2)

| GPS Pin | ESP32 Pin |
|---------|-----------|
| VCC | 3.3V |
| GND | GND |
| TX | GPIO 16 (RX2) |
| RX | GPIO 17 (TX2) |

#### Step 5: Connect DX-LR03 LoRa (Serial1)

| LoRa Pin | ESP32 Pin |
|----------|-----------|
| VCC | 3.3V |
| GND | GND |
| TX | GPIO 26 (RX1) |
| RX | GPIO 27 (TX1) |

---

### Ground Station (BW16) Assembly

#### Step 1: Connect LoRa Module (Serial1)

| LoRa Pin | BW16 Pin |
|----------|----------|
| VCC | 3.3V or 5V |
| GND | GND |
| TX | PB2 (RX1) |
| RX | PB1 (TX1) |

#### Step 2: Connect ST7920 LCD (SPI)

| LCD Pin | Pin Name | BW16 Pin |
|---------|----------|----------|
| E (Clock) | SCLK | PA25 |
| RW (Data) | SID | PA27 |
| RS (Select) | CS | PA26 |
| PSB | - | GND **[CRITICAL]** |
| VCC | - | 5V |
| GND | - | GND |

**Critical:** PSB must be connected to GND to select **serial mode**. This determines how the LCD communicates with the BW16.

#### Step 3: Connect Status LEDs

| LED Color | Anode Pin | Cathode |
|-----------|-----------|---------|
| Green (LoRa) | PA30 | GND (via 220Ω resistor) |
| Blue (SD) | PA7 | GND (via 220Ω resistor) |
| Red (Monitor) | PA12 | GND (via 220Ω resistor) |

#### Step 4: Connect Arduino Nano (UART Bridge)

| Signal | BW16 Pin | Nano Pin |
|--------|----------|----------|
| TX | PA7 | D0 (RX) |
| RX | PA8 | D1 (TX) |
| GND | GND | GND |

**⚠️ Voltage Warning:** Use a **logic level shifter** or **voltage divider** on Nano TX (D1) → BW16 RX (PA8) because Nano is 5V and BW16 is 3.3V.

---

### Data Logger (Arduino Nano) Assembly

#### Step 1: Connect SD Card Adapter (SPI)

| Adapter Pin | Nano Pin |
|-------------|----------|
| VCC | 5V |
| GND | GND |
| CS | D10 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |

#### Step 2: Connect Status LED

| LED | Anode Pin | Cathode |
|-----|-----------|---------|
| Blue (SD Write) | D9 | GND (via 220Ω resistor) |

#### Step 3: Connect UART from BW16

| Signal | Nano Pin | From BW16 Pin |
|--------|----------|---------------|
| RX | D0 | PA7 (with level shifter/divider) |
| TX | D1 | PA8 (with level shifter/divider) |
| GND | GND | GND |

---

### Power Distribution

For a lab setup with USB power:

1. **ESP32D:** Powered via USB (3.3V regulator on board)
2. **BW16:** Powered via USB-C (provides 5V)
3. **Arduino Nano:** Powered from BW16's 5V rail (or separate USB)
4. **LCD:** 5V from BW16
5. **LoRa modules:** 3.3V regulated (check module variant)
6. **SD card adapter:** 5V input (on-board regulator converts to 3.3V)

For a field setup with LiPo battery (on ESP32D):

1. **ESP32D:** Powered via LiPo battery (3.7–4.2V) with on-board regulator to 3.3V
2. **BW16:** Powered via USB-C power bank or line power (5V)
3. **Nano:** Powered from BW16's 5V or separate power bank

---

## Firmware Upload Guide

### Prerequisites

- Arduino IDE (v1.8.13 or later)
- USB-to-UART cables (CH340G or FT232RL based)
- Board definitions installed (ESP32, BW16, Arduino Nano)
- Required libraries installed (see README)

---

### Option 1: Upload to ESP32D (Transmitter)

1. **Connect USB cable to ESP32D**
   - ESP32D has a built-in USB-C or micro-USB port

2. **Open Arduino IDE**
   - File → Open → `CanSat/CanSat_ESP_32D/CanSat_ESP_32D.ino`

3. **Select Board & Port**
   - Tools → Board → ESP32 → **ESP32 Dev Module**
   - Tools → Port → `/dev/ttyUSB0` (or your port)
   - Tools → Upload Speed → **921600**

4. **Upload**
   - Click **Upload** button
   - Wait for `Uploading...` to complete

5. **Verify**
   - Open Serial Monitor (115200 baud)
   - You should see: `CANSAT MISSION READY - TRANSMITTER`

---

### Option 2: Upload to BW16 (Receiver)

1. **Connect USB cable to BW16**
   - BW16 may require a USB-to-UART adapter

2. **Open Arduino IDE**
   - File → Open → `Ground Station/Ground_Station_BW16/Ground_Station_BW16.ino`

3. **Select Board & Port**
   - Tools → Board → **BW16** (or Arduino Leonardo as fallback)
   - Tools → Port → `/dev/ttyUSB1` (or your port)
   - Tools → Upload Speed → **115200**

4. **Upload**
   - Click **Upload** button

5. **Verify**
   - Open Serial Monitor (115200 baud)
   - You should see: `--- GROUND STATION BOOTING (BW16) ---`

---

### Option 3: Upload to Arduino Nano (Logger)

1. **Connect USB cable to Nano**
   - Nano has a micro-USB port

2. **Open Arduino IDE**
   - File → Open → `Ground Station/Ground_Station_NANO/Ground_Station_NANO.ino`

3. **Select Board & Port**
   - Tools → Board → **Arduino Nano**
   - Tools → Processor → **ATmega328P**
   - Tools → Port → `/dev/ttyUSB2` (or your port)
   - Tools → Upload Speed → **115200**

4. **Upload**
   - Click **Upload** button

5. **Verify**
   - Open Serial Monitor (9600 baud)
   - You should see: `--- NANO BOOT ---`

---

### Troubleshooting Upload Issues

| Issue | Solution |
|-------|----------|
| **COM port not detected** | Install driver (CH340G/FT232RL), restart IDE |
| **Board not detected** | Ensure board definition installed (Tools → Boards Manager) |
| **Upload timeout** | Lower baud rate, check USB cable, try different port |
| **Compilation error** | Verify all required libraries installed |
| **Sketch too large** | Remove debug serial output, optimize code |

---

**Next Steps:**
- [Configure LoRa modules](#lora-module-configuration) (AT commands)
- [Check Serial Monitor output](#serial-monitor-debugging) (data validation)
- [Analyze SD card data](#sd-card-data-analysis) (data retrieval)
