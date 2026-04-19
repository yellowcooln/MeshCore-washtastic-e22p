# ProMicro Bootloader Upgrade and OTA Notes

This is the process for updating a ProMicro nRF52840 bootloader and then using the OTA-capable bootloader for BLE firmware upgrades.

This applies to:

- ProMicro nRF52840 boards
- WashTastic boards with a ProMicro installed

## What you need

- A ProMicro nRF52840
- USB cable
- Computer with USB file access
- The correct bootloader `.uf2`
- Nordic DFU app if you want OTA updates:
  - Android: [https://play.google.com/store/apps/details?id=no.nordicsemi.android.dfu](https://play.google.com/store/apps/details?id=no.nordicsemi.android.dfu)
  - iPhone: [https://apps.apple.com/us/app/nrf-device-firmware-update/id1624454660](https://apps.apple.com/us/app/nrf-device-firmware-update/id1624454660)

## Bootloader options

You can use either of these:

- Official Adafruit bootloader:
  - [https://github.com/adafruit/Adafruit_nRF52_Bootloader/releases](https://github.com/adafruit/Adafruit_nRF52_Bootloader/releases)
- OTAFIX bootloader:
  - Repo: [https://github.com/oltaco/Adafruit_nRF52_Bootloader_OTAFIX](https://github.com/oltaco/Adafruit_nRF52_Bootloader_OTAFIX)

Important:

- The official Adafruit bootloader is fine for normal USB flashing
- The official Adafruit bootloader does not work for the OTA update flow described in this doc
- If you want BLE OTA firmware updates, use the OTAFIX bootloader

At the time of writing, the newest ProMicro OTAFIX build from that repo is:

- [`update-promicro_nrf52840_bootloader-0.9.2-OTAFIX2.1-BP1.2_nosd.uf2`](https://github.com/oltaco/Adafruit_nRF52_Bootloader_OTAFIX/releases/download/0.9.2-OTAFIX2.1-BP1.2/update-promicro_nrf52840_bootloader-0.9.2-OTAFIX2.1-BP1.2_nosd.uf2)

## 1) Put the board into USB bootloader mode

1. Plug the ProMicro into USB.
2. Enter bootloader / UF2 drive mode by doing one of these:
   - press reset twice quickly, or
   - briefly short `RST` to `GND` twice quickly
3. The board should mount as a UF2 drive, commonly `NICENANO`.

<img width="678" height="120" alt="image" src="https://github.com/user-attachments/assets/d65d9b08-616d-4e8b-8320-c318f2ebef36" />

## 2) Check the current bootloader version

Open `INFO_UF2.TXT` on the mounted drive.

That file shows the current bootloader version.

<img width="400" height="149" alt="image" src="https://github.com/user-attachments/assets/f6f6c589-1839-4cbc-b801-de831fa73c95" />

## 3) Flash the bootloader

If you only want USB flashing, you can use the official Adafruit bootloader.

If you want OTA firmware updates over BLE, use the OTAFIX bootloader.

1. Download the bootloader `.uf2` you want to use.
2. Copy it onto the mounted UF2 drive.
3. Wait for the board to reboot automatically.

If needed, enter bootloader mode again and reopen `INFO_UF2.TXT` to confirm the version changed.

<img width="400" height="249" alt="image" src="https://github.com/user-attachments/assets/1bb8cb06-db3c-435e-a626-feb69a07bccd" />

## 4) Flash MeshCore firmware over USB the first time

After the bootloader update, flash your MeshCore firmware over USB using the MeshCore web flasher.

MeshCore releases for the ProMicro custom firmware are here:

- [https://github.com/yellowcooln/MeshCore-washtastic-e22p/releases](https://github.com/yellowcooln/MeshCore-washtastic-e22p/releases)

USB flash steps:

1. Open the MeshCore flasher:
   - [https://flasher.meshcore.co.uk/](https://flasher.meshcore.co.uk/)
2. Select `Custom Firmware`.
3. Choose the correct firmware file from the release page above.
4. For USB flashing, use the `.uf2`.
5. Flash the device.

File types:

- `.uf2` = USB flashing
- `.zip` = BLE OTA with the Nordic DFU app

## 5) OTA firmware upgrades over BLE

With the OTAFIX bootloader installed, BLE OTA upgrades work with the Nordic DFU app.

This does not work with the standard official Adafruit bootloader.

DFU app links:

- Android: [https://play.google.com/store/apps/details?id=no.nordicsemi.android.dfu](https://play.google.com/store/apps/details?id=no.nordicsemi.android.dfu)
- iPhone: [https://apps.apple.com/us/app/nrf-device-firmware-update/id1624454660](https://apps.apple.com/us/app/nrf-device-firmware-update/id1624454660)

Confirmed working flow:

1. Put the MeshCore node into OTA mode.
2. Open the Nordic DFU app.
3. Select the firmware `.zip`, not the `.uf2`.
4. Connect to the OTA advertising device and start the update.

For repeater or room server builds, start OTA mode from the CLI first:

```text
start ota
```

The device should then advertise as something like `ProMicro_OTA`.

## Nordic DFU app settings that worked

These settings were tested and confirmed working with the OTAFIX bootloader:

- `Packet Receipt Notification`: `ON`
- `Number of packets`: `8`
- `Reboot time`: `0 ms`
- `Scan timeout`: `2000 ms`
- `Request high MTU`: `ON`
- `Disable resume`: `ON`
- `Prepare object delay`: `0 ms`
- `Force scanning`: `ON`
- `Keep bond information`: `OFF`
- `External MCU DFU`: `OFF`

If a phone is flaky with `Request high MTU`, retry once with it turned `OFF`.

## Notes

- OTA uses the firmware `.zip`, not the `.uf2`
- USB bootloader updates use the bootloader `.uf2`
- If OTA fails repeatedly, move the phone closer to the node and retry
- If the board does not show the OTA device name, verify the OTAFIX bootloader is installed first
