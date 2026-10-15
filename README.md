# MeshCore Washtastic E22P Firmware

Boston Mesh has a custom batch of Washtastic boards that use the E22P module (not the standard E22). Because of that hardware difference, we built a dedicated firmware variant with the required changes documented here: [commit](https://github.com/yellowcooln/MeshCore-washtastic-e22p/commit/3ebd5f18f94eb0c87fe2e88f7eaa14b53440a2b4)

In the release uploads, I've also included the additional firmware changes needed to run the device as an MQTT Observer. This build enables packet logging via `-D MESH_PACKET_LOGGING=1`, set through `PLATFORMIO_BUILD_FLAGS`.

![PXL_20251231_1801422752](https://github.com/user-attachments/assets/8eeb6797-cc85-424d-8826-65eec10f9227)

## Bootloader Update

We need to update the bootloader on the ProMicros. Make sure to follow [promicro-upgrade.md](https://github.com/yellowcooln/MeshCore-washtastic-e22p/blob/v1.18.0-washtastic/promicro-upgrade.md) before flashing the custom firmware.

## MeshCore v1.18.0 builds

This branch is based on the upstream companion, repeater, and room-server v1.18.0 release tags (all point to the same source commit).

The **Build Washtastic Firmwares** workflow runs manually on GitHub-hosted `ubuntu-latest` runners, not self-hosted runners. Select `v1.18.0-washtastic` and set `firmware_version` to `v1.18.0`.

It produces UF2 and DFU ZIP files for Companion BLE, Companion USB, Repeater, Repeater logging, Room Server, and Room Server logging. Only the logging repeater/room-server builds enable `MESH_PACKET_LOGGING` for MQTT observers.

To build locally with PlatformIO installed:

```bash
FIRMWARE_VERSION=v1.18.0 bash scripts/build-washtastic-firmwares.sh
```

Outputs are written to `out/`.

## Download

You can find all the downloads in the [Releases Page](https://github.com/yellowcooln/MeshCore-washtastic-e22p/tags).
