# LoRa Slave AT Session (BW16)

This file turns the raw Serial Monitor trace in `Ground Station/Ground_Station_BW16/LoRa_Slave.txt` into a readable “session” format.

Source (raw trace):
- `Ground Station/Ground_Station_BW16/LoRa_Slave.txt`

---

## 0) AT mode negotiation (as captured)

The trace shows the following sequence pattern:

```text
+++           (enter AT)
AT+HELP       (print LoRa Parameter block)
AT+CHANNEL03  (set channel)
AT+MAC00,01   (set MAC)
+++           (exit AT)
```

---

## 1) Session: inspect parameters + channel + MAC

### Step 1 — inspect parameters
- `+++`
- `AT+HELP`

From the trace’s `LoRa Parameter:` block:
- `MODE:0`
- `LEVEL:0 >> 244.14` (also appears as `244.14` / `244.140625bps`)
- `SLEEP:2`
- `Frequency:4372` / `437200000hz`
- `MAC:00,01`
- `Spreading Factor:12`
- `Coding rate:2`
- `CRC:0(false)`
- `Preamble:8` (present in multiple HELP outputs)
- `IQ:0(false)`
- `Power:5`

### Step 2 — set channel
- `AT+CHANNEL03`
- response: `+CHANNEL=03` and `OK`

### Step 3 — set MAC
- `AT+MAC00,01`
- response: `+MAC=00,01` and `OK`

---

## 2) Session: LEVEL tuning attempt

The trace later shows:

- `AT+LEVEL` → `+LEVEL=0`
- `AT+LEVEL7` → `OK`

After `AT+LEVEL7`, a subsequent `AT+HELP` output reports:
- `LEVEL:7 >> 13020.833984bps`
- `Spreading Factor:5`
- `Coding rate:2`

> If you change LEVEL on the slave, apply the same LEVEL on the master, otherwise reception may fail.

---

## 3) “Known-good” set for this repo (from the trace)

Use the parameter set matching the `LEVEL:0` / `Spreading Factor:12` part of the log:

- MAC: `00,01`
- Channel: `03`
- Frequency: around `4372 MHz`
- LEVEL: `0`
- Spreading Factor: `12`
- Coding rate: `2`
- CRC: `false`
- Preamble: `8`

