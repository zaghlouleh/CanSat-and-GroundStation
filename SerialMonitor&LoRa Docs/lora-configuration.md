# LoRa AT Configuration (Master + Slave)

This page consolidates the LoRa AT settings discovered in the repository’s raw debug traces:

- `CanSat/CanSat_ESP_32D/LoRa_Master.txt`
- `Ground Station/Ground_Station_BW16/LoRa_Slave.txt`

> ⚠️ Notes
> - The AT command set can differ slightly between LoRa module firmware versions.
> - The values below are what the traces show the modules reporting via `AT+HELP`.
> - Treat this as “known-good configuration for this project”, not as a universal LoRa modem manual.

---

## 1) One-time “pairing” parameters

Both sides must match on the parameters that determine reception/decoding.

| Parameter | AT command shown in traces | Value shown in traces | Used for |
|---|---|---:|---|
| MAC / address | `AT+MAC00,01` | `00,01` | addressing / compatibility |
| Channel | `AT+CHANNEL03` | `03` | selecting the radio “channel” |

---

## 2) Physical-layer parameters (as reported by `AT+HELP`)

From the `LoRa Parameter:` block (output of `AT+HELP`).

| Parameter | Value shown in traces | Notes |
|---|---:|---|
| MODE | `0` | project log shows `MODE:0` |
| LEVEL | `0` (later also `7`) | `LEVEL` affects symbol rate / bitrate; see section 3 |
| SLEEP | `2` | power/sleep setting |
| Frequency | around `4372` MHz / `437200000hz` | traces show both forms (unit formatting differs) |
| Spreading Factor | `12` (later also `5`) | `Spreading Factor` |
| Coding rate | `2` | `Coding rate:2` |
| CRC | `0(false)` | CRC disabled in the log |
| Preamble | `8` | `Preamble:8` |
| IQ | `0(false)` | IQ setting |
| Power | `5` | `Power:5` |

---

## 3) LEVEL / Spreading Factor variants seen in the logs

The raw traces show multiple “attempts”:

### Variant A (most consistent with default telemetry attempts)
- `AT+LEVEL` → `+LEVEL=0`
- `AT+HELP` reports:
  - `Spreading Factor:12`
  - `Frequency:4372`
  - `Coding rate:2`
  - `CRC:0(false)`
  - `Preamble:8`

### Variant B (later tuning attempt)
- `AT+LEVEL7` → `OK`
- `AT+HELP` reports:
  - `LEVEL:7 >> 13020.833984bps`
  - `Frequency:43` (this line appears truncated/short in the raw trace; another line shows `7200000hz >> 03`)
  - `Spreading Factor:5`
  - `Coding rate:2`

> Recommendation for project use: keep both ends aligned to **the same variant**.

---

## 4) Practical checklist (copy/paste)

1. Enter AT mode (log shows `+++`).
2. Apply the shared addressing settings on **both** modules:
   - `AT+MAC00,01`
   - `AT+CHANNEL03`
3. Apply the radio PHY settings consistently on **both** modules:
   - keep `LEVEL` the same on both ends
   - verify via `AT+HELP` that you see the expected:
     - Frequency (around 4372 MHz)
     - Spreading Factor
     - Coding rate
     - CRC
     - Preamble
4. Exit AT mode (`Exit AT` in logs follows the `+++` sequence).

---

## 5) Where the “truth” comes from

This documentation is derived from the module’s own responses in the raw logs (specifically the `AT+HELP` output sections labeled `LoRa Parameter:`).

- Master session: [`docs/lora-master-at-session.md`](./lora-master-at-session.md)
- Slave session: [`docs/lora-slave-at-session.md`](./lora-slave-at-session.md)

