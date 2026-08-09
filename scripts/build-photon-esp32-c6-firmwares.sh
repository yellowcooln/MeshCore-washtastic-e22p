#!/usr/bin/env bash

set -euo pipefail

if [ -z "${FIRMWARE_VERSION:-}" ]; then
  echo "FIRMWARE_VERSION must be set"
  exit 1
fi

OUTPUT_DIR="${OUTPUT_DIR:-out}"
FIRMWARE_BUILD_DATE="$(date '+%d-%b-%Y')"
PHOTON_FIRMWARE_VERSION="${FIRMWARE_VERSION}-Photon-ESP32-C6"

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

build_photon_variant() {
  local env_name="$1"
  local asset_name="$2"
  local extra_flags="${3:-}"
  local build_flags="${PLATFORMIO_BUILD_FLAGS:-}"

  build_flags="${build_flags} -DFIRMWARE_BUILD_DATE='\"${FIRMWARE_BUILD_DATE}\"'"
  build_flags="${build_flags} -DFIRMWARE_VERSION='\"${PHOTON_FIRMWARE_VERSION}\"'"

  if [ -n "${extra_flags}" ]; then
    build_flags="${build_flags} ${extra_flags}"
  fi

  echo "Building ${env_name} -> ${asset_name}-${FIRMWARE_VERSION} (internal ${PHOTON_FIRMWARE_VERSION})"
  rm -rf ".pio/build/${env_name}"

  PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -e "${env_name}"
  PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -t mergebin -e "${env_name}"

  cp ".pio/build/${env_name}/firmware.bin" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}.bin"
  cp ".pio/build/${env_name}/firmware-merged.bin" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}-merged.bin"
}

build_photon_variant "photon_esp32_c6_companion_radio_ble" "Photon-ESP32-C6-Companion-BLE"
build_photon_variant "photon_esp32_c6_companion_radio_usb" "Photon-ESP32-C6-Companion-USB"
build_photon_variant "photon_esp32_c6_repeater" "Photon-ESP32-C6-Repeater"
build_photon_variant "photon_esp32_c6_room_server" "Photon-ESP32-C6-Room-Server"

printf 'Built files:\n'
printf '  %s\n' "${OUTPUT_DIR}"/*
