#!/usr/bin/env bash

set -euo pipefail

if [ -z "${FIRMWARE_VERSION:-}" ]; then
  echo "FIRMWARE_VERSION must be set"
  exit 1
fi

OUTPUT_DIR="${OUTPUT_DIR:-out}"

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

build_photon_variant() {
  local env_name="$1"
  local asset_name="$2"
  local asset_suffix="${3:-}"
  local extra_flags="${4:-}"

  echo "Building ${env_name} -> ${asset_name}-${FIRMWARE_VERSION}${asset_suffix}"

  rm -rf ".pio/build/${env_name}"

  if [ -n "${extra_flags}" ]; then
    PLATFORMIO_BUILD_FLAGS="${extra_flags}" FIRMWARE_VERSION="${FIRMWARE_VERSION}" bash build.sh build-firmware "${env_name}"
  else
    FIRMWARE_VERSION="${FIRMWARE_VERSION}" bash build.sh build-firmware "${env_name}"
  fi

  cp ".pio/build/${env_name}/firmware.uf2" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}${asset_suffix}.uf2"
  cp ".pio/build/${env_name}/firmware.zip" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}${asset_suffix}.zip"
}

build_photon_variant "meshsmith_photon_e22p_30dbm_companion_radio_ble" "Photon-Companion-BLE"
build_photon_variant "meshsmith_photon_e22p_30dbm_companion_radio_usb" "Photon-Companion-USB"
build_photon_variant "meshsmith_photon_e22p_30dbm_repeater" "Photon-Repeater"
build_photon_variant "meshsmith_photon_e22p_30dbm_repeater" "Photon-Repeater" "-logging" "-DMESH_PACKET_LOGGING=1"
build_photon_variant "meshsmith_photon_e22p_30dbm_room_server" "Photon-Room-Server"
build_photon_variant "meshsmith_photon_e22p_30dbm_room_server" "Photon-Room-Server" "-logging" "-DMESH_PACKET_LOGGING=1"

echo "Built files:"
find "${OUTPUT_DIR}" -maxdepth 1 -type f | sort
