# MeshCore Washtastic E22P Firmware

Boston Mesh has a custom batch of Washtastic boards that use the E22P module (not the standard E22). Because of that hardware difference, we built a dedicated firmware variant with the required changes documented here: [commit](https://github.com/yellowcooln/MeshCore-washtastic-e22p/commit/3ebd5f18f94eb0c87fe2e88f7eaa14b53440a2b4)

In the release uploads, I’ve also included the additional firmware changes needed to run the device as an MQTT Observer. This build enables packet logging via `-D MESH_PACKET_LOGGING=1`, set through `PLATFORMIO_BUILD_FLAGS` 

# Download

You can find all the downloads in the [Releases Page](https://github.com/yellowcooln/MeshCore-washtastic-e22p/tags).
