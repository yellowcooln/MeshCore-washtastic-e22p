# RAK4631 v1.17.1 BatteryInfo + power-saving repeater

Base: upstream `repeater-v1.17.1` (`d9296435`).
Target: `RAK_4631_batteryinfo_powersaving_repeater`.

This is the standard RAK4631/WisBlock SX1262 repeater, not an Ethernet,
companion, or RAK3401/RAK10724 build. The upstream RAK4631 pin map, battery
ADC calibration, radio defaults, and bootloader layout are unchanged.
The normal `RAK_4631_repeater` environment remains available without the
new CLI/preferences or BatteryInfo transmissions.

## Behavior

- Manual and scheduled flood adverts also enqueue a BatteryInfo group message
  on the same BatteryInfo channel used by the fork's existing Photon builds.
- Local/zero-hop adverts do not trigger a BatteryInfo message.
- The report includes battery volts, estimated percentage, temperature and
  optional environmental readings. Missing environmental readings are `na`.
  Temperature falls back to the MCU die temperature, not ambient temperature.
- Percentage is a linear estimate between 3.0 V (0%) and 4.2 V (100%), clamped
  to that range. It is not a fuel-gauge measurement. Other battery chemistries
  need a different voltage curve.
- BatteryInfo text uses an unscoped flood and the configured path-hash width.
  Advertisements retain upstream scoped-flood routing. The BatteryInfo channel
  is shared, not a private telemetry channel.
- RX duty cycling includes packet-metadata preservation, TX/CAD sequencing,
  periodic continuous-RX noise calibration, and soft/hard radio recovery.
- RXPS settings persist using the v1.17.1 JSON config serializer. Existing
  settings and upstream MCU power-saving behavior are preserved.

## Enable after flashing

Both MCU power saving and RX duty cycling are off by default. Flashing does
not override saved node settings. Use the repeater CLI:

```text
powersaving on
set radio.rxps conservative
powersaving
get radio.rxps
get rxps.wd
```

`conservative` selects RXPS level 1. For additional savings, test
`set radio.rxps balanced` (level 5), or `set radio.rxps level 1` through
`set radio.rxps level 10`. Profiles retune when SF/BW changes. Manual timings
use `set radio.rxps <rx_us> <sleep_us>`; the SX1262 minimum sleep is 6016 us.
Duty cycling can reduce reception reliability with incompatible preambles;
start with conservative and measure before increasing it.

Disable independently:

```text
set radio.rxps off
powersaving off
```

Use `advert` to exercise the flood report and `advert.zerohop` for the negative
case. Scheduled reports follow the existing flood-advert interval; no new
periodic report timer is added. Configure the node name, administrator password,
radio region/frequency, and advert interval before deploying. A fresh node
inherits upstream defaults; this build does not hard-code deployment settings.

## Build and package

```sh
python3 scripts/build-rak4631-batteryinfo.py --output /path/to/output
```

This runs the exact PlatformIO target, generates UF2, and packages the UF2,
serial/BLE DFU ZIP, Intel HEX, ELF, source provenance, and SHA256SUMS.txt outside
tracked firmware directories. Check hashes from inside the output directory:

```sh
sha256sum -c SHA256SUMS.txt
```

UF2 is for a compatible RAK4631 UF2 bootloader (application starts at 0x26000,
nRF52840 family ID 0xADA52840). The DFU ZIP is an application update, not a
replacement bootloader or SoftDevice. Do not use these files on RAK4631-R
AT firmware or other core modules without first establishing bootloader
compatibility. No hardware flashing is performed by the packaging script.

## Verification and limits

Built the custom RAK4631 target, ordinary RAK4631 repeater, and MeshTracker X1
LR2021 repeater. Native RXPS timing, BatteryInfo trigger/voltage, and config
serializer tests pass. UF2/DFU package structure and checksums are validated.

These are source/build/package checks, not live-node proof. Current draw,
real battery calibration, RF receive reliability, flood-only report behavior,
and watchdog recovery still require testing on the actual RAK4631.
