# Photon nRF52 BatteryInfo + PowerSaving v18

Base: IoTThinks/MeshCore `PowerSaving-v18`, exact commit
`6beb4506dc354b80178fe10477756d8a2ab2ce6f` (MeshCore v1.18.0).
Branch: `PowerSaving-v18-meshsmith-photon-nrf52`.
Target: `meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater`.
Firmware version: `v1.18.0-PS18-Photon-nRF-BattPS`.

This carries the fork's existing XIAO nRF52840 Photon-1W hardware support
forward onto v18. It is not Photon ESP32-C6 firmware. E22P drive remains
20 dBm (nominal 30 dBm PA output), with the existing D0/D1/D2/D3 radio mapping,
1.8 V TCXO and DIO2 RF switch. Actual RF output/legal limits need verification.
The bootloader/SoftDevice is not replaced: S140 7.3.0, application origin 0x27000,
application limit 0xED000, UF2 family 0xADA52840. Do not flash the RAK build here.

## Reports

Successful manual and scheduled flood adverts enqueue one report to the same
`#batteryinfo` channel/key as our previous fork builds. The current upstream
scoped advert routing is retained. The side message keeps the previous unscoped
flood and configured path-hash width. Boot/local/zero-hop adverts do not trigger
reports. No new RF timer is added.

Example format (illustrative, not a hardware measurement):
`battery=3.60v 50% charge=+2.00%/hr temp=77.0F`

Voltage is read from the MAX17048 VCELL register. Percentage retains our rounded,
clamped linear 3.0–4.2 V estimate, not the gauge's SOC register. Charge rate is
signed MAX17048 CRATE percent per hour, **not current or watts**. Temperature is
the nRF52840 chip die sensor in Fahrenheit, **not ambient temperature**. Standard
binary telemetry still uses its existing units. Failed/unavailable charge rate
or temperature is omitted; an unreadable gauge reports `battery=na`. This
sensorless channel report does not emit empty humidity/pressure/altitude fields
or actively probe external sensors. Reports that cannot fit the full body are
dropped rather than truncating units.

## Setup

Upstream v18 fresh repeater defaults enable MCU power saving and RXPS level 6
(preamble 16). Existing saved settings override defaults; flashing is not a
factory reset. Inspect and configure the actual device:

```text
ver
powersaving
get radio.rxps
get flood.advert.interval
```

Use upstream controls if needed:

```text
powersaving on
set radio.rxps balanced
advert
advert.zerohop
```

`advert` should produce one report; `advert.zerohop` must produce no report or
flood. Scheduled reports follow the existing flood advert interval (fresh
upstream default 47 hours). For a three-hour cadence use
`set flood.advert.interval 3`. No independent battery-report timer is added.
Follow upstream instructions to reboot after changing RXPS settings. Configure
the legal network frequency, BW/SF/CR, TX settings, node name and administrator
password before deployment: inherited RF defaults are 869.618 MHz/BW62.5/SF8,
not a region-specific deployment preset.

## Build and verify

```sh
pio run -e meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater
pio run -e meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater -t create_uf2
g++ -std=c++17 -Wall -Wextra -Werror -Isrc scripts/tests/photon_battery_report_test.cpp -o /path/to/report-test
/path/to/report-test
python3 scripts/check_batteryinfo_routing.py
pio test -e native
python3 scripts/package_photon_nrf52.py --output /path/to/package
```

`firmware.zip` inside `.pio/build/<target>` is the application DFU ZIP accepted
by the MeshCore web flasher's custom-file input. Upload that inner firmware ZIP,
not an outer download bundle. UF2 is optional compatible-bootloader drive-copy
recovery; HEX/ELF are not browser-flasher inputs. Preserve existing identity and
settings and the physical bootloader recovery route.

Build/native/routing/artifact verification does not establish RF delivery,
receive reliability, actual current reduction, gauge accuracy, or physical
bootloader compatibility. Those require a real Photon. No device is flashed by
the build/package scripts.
