# Serial Monitor output & data framing (tested drafts)

This document explains what you should see in the Serial Monitor for each board, and how the data is transformed at each hop.

It is based on the repository draft logs:

- `CanSat ESP32D/CanSat_ESP_32D_Displaying_Serial_Monitor.txt`
- `Ground Station BW16/Ground_Station_BW16_Displaying_Serial_Monitor.txt`
- `Ground Station NANO/Ground_Station_NANO_Displaying_Serial_Monitor.txt`
- `Ground Station NANO/Ground_Station_NANO_Save_SD_Card.txt`

---

## 1) ESP32D (Transmitter): Serial Monitor = “live telemetry table”

### Example (from draft)

From `CanSat ESP32D/CanSat_ESP_32D_Displaying_Serial_Monitor.txt`, the Serial Monitor repeats blocks like:

```text
+------------------------------------------------------------------------------+
|                        CANSAT LIVE TELEMETRY TABLE                           |
+----------------------+---------------------------+---------------------------+
| DHT22                | Humidity                  | 37.80 %                   |
| DHT22                | Temperature               | 25.70 C                   |
| BMP180               | Pressure                  | 93099 Pa                  |
| BMP180               | Altitude (Est)            | 707.93 m                  |
| MPU6050              | Accel X / Y / Z           | -0.5, -0.4, 11.0          |
| MPU6050              | Gyro X / Y / Z            | -0.1, 0.0, -0.0           |
| NEO-6M GPS           | Fix Status                | NO FIX / SEARCHING        |
| DX-LR03-LoRa         | Radio Status              | ONLINE (TX OK)            |
| DX-LR03-LoRa         | Last Packet Size          | 62 Bytes                  |
| DX-LR03-LoRa         | Frequency Band            | 433 MHz                   |
+------------------------------------------------------------------------------+
[STATUS] CanSat transmitter is operational
```

File reference: `CanSat ESP32D/CanSat_ESP_32D_Displaying_Serial_Monitor.txt`

### What you see
- A repeating table labeled **`CANSAT LIVE TELEMETRY TABLE`**.
- Typical rows include:
  - `DHT22`: humidity, temperature
  - `BMP180`: pressure, altitude estimate
  - `MPU6050`: accel / gyro values and sensor temperature
  - `NEO-6M GPS`: fix status
  - `DX-LR03-LoRa`: radio status, last packet size, frequency band

### What it means
This Serial Monitor output is the *transmitter-side view* of the sensor readings that will later be packed into the LoRa frame.

### Data framing used on LoRa
The board sends one ASCII line per telemetry frame over LoRa:

- Prefix: `CS1,`
- Comma-separated fields in a fixed order (terminated by `\n`)

The receiver only processes lines that start with `CS1,`.

---

## 2) BW16 (Receiver + LCD + bridge): Serial Monitor = boot handshake + “PKT” forwarding

File reference: `Ground Station BW16/Ground_Station_BW16_Displaying_Serial_Monitor.txt`

### Example (boot + forwarding)

From `Ground Station BW16/Ground_Station_BW16_Displaying_Serial_Monitor.txt`, you should see a boot block like:

```text
--- GROUND STATION BOOTING (BW16) ---
[OK] LoRa UART (Serial1) Initialized.
BW16_HELLO
SD_STATUS
[SD] Status changed: READY
[STATION ALIVE] Total Packets: 0
```

And then, when telemetry arrives, BW16 forwards a single line to the Nano:

```text
PKT,60,0,23.00,40.60,93366.00,684.70,0.000000,0.000000,-0.60,-0.50,11.00,4.06
```

### What you see on boot
The draft shows a boot sequence similar to:

- `--- GROUND STATION BOOTING (BW16) ---`
- `BW16_HELLO` / `SD_STATUS` commands being sent to the Nano
- `SD` status changing (example: `[SD] Status changed: READY`)
- Then `[STATION ALIVE] Total Packets: N`

### What you see while receiving
BW16 prints a repeating **`GROUND STATION LIVE TELEMETRY TABLE`**.

This table is the *post-parse* view, i.e. values extracted from the LoRa line and formatted for display.

Example of what also appears:
- `DX-LR03-LoRa`: last packet size and packet number
- `Nano SD Logger`: card status (e.g. `VIA NANO`)

### The exact command forwarded to the Nano
Each successfully parsed LoRa packet is forwarded as a single line to the Nano:

```text
PKT,<packetSize>,<packetNum>,<t>,<hum>,<press>,<alt>,<lat>,<lng>,<ax>,<ay>,<az>,<tilt>
```

Where:
- `packetSize` = length of the incoming LoRa line (`data.length()` on BW16)
- `packetNum` = the receiver-side packet counter
- `t/hum/press/alt/lat/lng/ax/ay/az` are parsed from the original `CS1,` payload
- `tilt` is computed from `ax, ay, az` (see receiver firmware)

### What to look for in the draft
In the draft log, you will see lines like:

- `PKT,60,0,23.00,40.60,...`
- `SD_STATUS` polling happening between packets

---

## 3) Arduino Nano (Logger): Serial Monitor = SD handshake + “save table” print

File reference: `Ground Station NANO/Ground_Station_NANO_Displaying_Serial_Monitor.txt`

### Example (boot/handshake)

```text
--- NANO BOOT ---
SD_CARD: OK
NANO_READY
SD_READY
```

### What you see on boot
The draft shows a handshake like:

- `SD_CARD: OK`
- `NANO_READY`
- `SD_READY`

### What you see when BW16 sends `PKT,...`
For every incoming packet, the Nano prints:

- `GROUND STATION SAVE TELEMETRY TABLE` (a human-readable table)
- then write status, for example:
  - `SD_WRITE_OK` when appended to `cansat.txt`
  - `SD_WRITE_ERR` if append failed

---

## 4) microSD file format (cansat.txt): “Time:” blocks + separator

File reference: `Ground Station NANO/Ground_Station_NANO_Save_SD_Card.txt`

### What you see in the file
The draft shows the contents written to `cansat.txt`.

Each entry is a block formatted like:

1. `Time: <millis>`
2. `GROUND STATION SAVE TELEMETRY TABLE` block (same table printed to Serial)
3. a separator line consisting of `---`

Example blocks look like:

```text
Time: 14894

+------------------------------------------------------------------------------+
|                    GROUND STATION SAVE TELEMETRY TABLE                       |
...
---
Time: 17073
...
```

So the SD file becomes a sequence of timestamped blocks for later analysis.

---

## Quick “mental model” of the pipeline

1. **ESP32D** prints a live table and sends `CS1,<...>\n` over LoRa.
2. **BW16** receives `CS1,<...>\n`, parses values, updates LCD, and prints the parsed table.
3. **BW16** forwards parsed data to Nano as `PKT,<...>`.
4. **Nano** prints the save table and appends the same formatted block to `cansat.txt` on microSD.

