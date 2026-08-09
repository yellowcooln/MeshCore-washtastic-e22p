#!/usr/bin/env bash

set -euo pipefail

if [ -z "${FIRMWARE_VERSION:-}" ]; then
  echo "FIRMWARE_VERSION must be set"
  exit 1
fi
if [[ ! "${FIRMWARE_VERSION}" =~ ^[A-Za-z0-9][A-Za-z0-9._-]*$ ]]; then
  echo "FIRMWARE_VERSION may contain only letters, numbers, dots, underscores, and dashes"
  exit 1
fi

OUTPUT_DIR="${OUTPUT_DIR:-out}"
if [ -z "${OUTPUT_DIR}" ] || [ "${OUTPUT_DIR}" = "/" ]; then
  echo "Refusing unsafe OUTPUT_DIR"
  exit 1
fi
FIRMWARE_BUILD_DATE="$(date '+%d-%b-%Y')"
PHOTON_FIRMWARE_VERSION="${FIRMWARE_VERSION}-Photon-nRF52"

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

build_photon_variant() {
  local env_name="$1"
  local asset_name="$2"
  local asset_suffix="${3:-}"
  local extra_flags="${4:-}"
  local internal_version="${5:-${PHOTON_FIRMWARE_VERSION}}"
  local build_flags=""

  echo "Building ${env_name} -> ${asset_name}-${FIRMWARE_VERSION}${asset_suffix} (internal ${internal_version})"

  rm -rf ".pio/build/${env_name}"

  build_flags="${PLATFORMIO_BUILD_FLAGS:-}"
  build_flags="${build_flags} -DFIRMWARE_BUILD_DATE='\"${FIRMWARE_BUILD_DATE}\"'"
  build_flags="${build_flags} -DFIRMWARE_VERSION='\"${internal_version}\"'"

  if [ -n "${extra_flags}" ]; then
    build_flags="${build_flags} ${extra_flags}"
  fi

  PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -e "${env_name}"
  python3 bin/uf2conv/uf2conv.py ".pio/build/${env_name}/firmware.hex" -c -o ".pio/build/${env_name}/firmware.uf2" -f 0xADA52840

  cp ".pio/build/${env_name}/firmware.uf2" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}${asset_suffix}.uf2"
  cp ".pio/build/${env_name}/firmware.zip" "${OUTPUT_DIR}/${asset_name}-${FIRMWARE_VERSION}${asset_suffix}.zip"
}

build_photon_variant "meshsmith_photon_nrf52_e22p_30dbm_companion_radio_ble" "Photon-nRF52-Companion-BLE"
build_photon_variant "meshsmith_photon_nrf52_e22p_30dbm_companion_radio_usb" "Photon-nRF52-Companion-USB"
build_photon_variant "meshsmith_photon_nrf52_e22p_30dbm_repeater" "Photon-nRF52-Repeater"
build_photon_variant "meshsmith_photon_nrf52_e22p_30dbm_repeater" "Photon-nRF52-Repeater" "-logging" "-DMESH_PACKET_LOGGING=1"
build_photon_variant \
  "meshsmith_photon_nrf52_e22p_30dbm_batteryinfo_powersaving_repeater" \
  "Photon-nRF52-BatteryInfo-PowerSaving-Repeater" \
  "" "" "${FIRMWARE_VERSION}-Photon-nRF-BattPS"
build_photon_variant "meshsmith_photon_nrf52_e22p_30dbm_room_server" "Photon-nRF52-Room-Server"
build_photon_variant "meshsmith_photon_nrf52_e22p_30dbm_room_server" "Photon-nRF52-Room-Server" "-logging" "-DMESH_PACKET_LOGGING=1"

(
  cd "${OUTPUT_DIR}"
  sha256sum ./*.uf2 ./*.zip > SHA256SUMS.txt
)

printf 'Built files:\n'
printf '  %s\n' "${OUTPUT_DIR}"/*
