#!/usr/bin/env bash

set -euo pipefail

if [ -z "${FIRMWARE_VERSION:-}" ]; then
  echo "FIRMWARE_VERSION must be set"
  exit 1
fi

OUTPUT_DIR="${OUTPUT_DIR:-out}"
COMMIT_HASH="$(git rev-parse --short HEAD)"
FIRMWARE_BUILD_DATE="$(date '+%d-%b-%Y')"
BUILD_VERSION="${FIRMWARE_VERSION}-${COMMIT_HASH}"

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

build_promicro_variant() {
  local env_name="$1"
  local asset_name="$2"
  local asset_suffix="${3:-}"
  local extra_flags="${4:-}"
  local build_flags=""

  build_flags="${PLATFORMIO_BUILD_FLAGS:-}"
  build_flags="${build_flags} -DFIRMWARE_BUILD_DATE='\"${FIRMWARE_BUILD_DATE}\"'"
  build_flags="${build_flags} -DFIRMWARE_VERSION='\"${BUILD_VERSION}\"'"

  if [ -n "${extra_flags}" ]; then
    build_flags="${build_flags} ${extra_flags}"
  fi

  echo "Building ${env_name} -> ${asset_name}-${FIRMWARE_VERSION}${asset_suffix}"

  rm -rf ".pio/build/${env_name}"

  PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -e "${env_name}"
  python3 bin/uf2conv/uf2conv.py ".pio/build/${env_name}/firmware.hex" -c -o ".pio/build/${env_name}/firmware.uf2" -f 0xADA52840

  cp ".pio/build/${env_name}/firmware.uf2" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}${asset_suffix}.uf2"
  cp ".pio/build/${env_name}/firmware.zip" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}${asset_suffix}.zip"
}

build_promicro_variant "ProMicro_companion_radio_ble" "Companion-BLE"
build_promicro_variant "ProMicro_companion_radio_usb" "Companion-USB"
build_promicro_variant "ProMicro_repeater" "Repeater"
build_promicro_variant "ProMicro_repeater" "Repeater" "-logging" "-DMESH_PACKET_LOGGING=1"
build_promicro_variant "ProMicro_room_server" "Room-Server"
build_promicro_variant "ProMicro_room_server" "Room-Server" "-logging" "-DMESH_PACKET_LOGGING=1"

echo "Built files:"
find "${OUTPUT_DIR}" -maxdepth 1 -type f | sort
