# ProMicro Bootloader Update

Some ProMicro boards ship with an older bootloader (for example, `0.6.0`). This guide walks through upgrading to a newer version (example shown: `0.9.2`).

## What you’ll need

- A ProMicro (or Washtastic board with a ProMicro installed)
- A USB cable + a computer
- The latest bootloader `.uf2` file

## 1) Download the latest bootloader

Download the newest bootloader file from here:  
https://github.com/gargomoma/fakeTec_pcb?tab=readme-ov-file#bootloader

At the time of writing, the newest bootloader is:

- `update-nice_nano_bootloader-0.9.2_nosd.uf2`

## 2) Put the device into USB Storage mode

1. Connect the ProMicro to your computer.
2. Enter **USB Storage mode** (it should mount as a USB drive named **NICENANO**) using either method:
   - Press the reset button **twice quickly** on the Washtastic board, **or**
   - **Double-tap** by bridging **RST** and **GND** pins (like the reset button)

You should see a USB device named **NICENANO**.

<img width="678" height="120" alt="image" src="https://github.com/user-attachments/assets/d65d9b08-616d-4e8b-8320-c318f2ebef36" />

## 3) Check the currently installed version

Open `INFO_UF2.TXT` on the **NICENANO** drive to see the currently installed bootloader version.

<img width="400" height="149" alt="image" src="https://github.com/user-attachments/assets/f6f6c589-1839-4cbc-b801-de831fa73c95" />

## 4) Flash the new bootloader

Copy the new `.uf2` file (example: `update-nice_nano_bootloader-0.9.2_nosd.uf2`) onto the **NICENANO** drive.

The device should reboot automatically.

## 5) Verify the update

If it doesn’t reboot back into **NICENANO** mode:

1. Press reset **twice quickly** again to re-enter USB Storage mode.
2. Re-open `INFO_UF2.TXT` and confirm it shows `0.9.2` (or whatever the latest version is at the time).

<img width="740" height="280" alt="image" src="https://github.com/user-attachments/assets/e03b96c2-8bdf-4049-8bec-53c63a86318b" />

## 6) Reboot and flash MeshCore firmware

1. Unplug the ProMicro from USB.
2. Plug it back in.
3. Open the **[MeshCore Web Flasher](https://flasher.meshcore.co.uk/)**.
4. Download the correct firmware from this repo’s **[Releases page](https://github.com/yellowcooln/MeshCore-washtastic-e22p/releases)** and flash it using the web flasher.

