#!/usr/bin/env bash

set -euo pipefail

if [ -z "${FIRMWARE_VERSION:-}" ]; then
  echo "FIRMWARE_VERSION must be set"
  exit 1
fi

OUTPUT_DIR="${OUTPUT_DIR:-out}"
FIRMWARE_BUILD_DATE="$(date '+%d-%b-%Y')"
PHOTON_FIRMWARE_VERSION="${FIRMWARE_VERSION}-Photon-ESP32-C6-KISS-TCP"
ENV_NAME="photon_esp32_c6_kiss_tcp_modem"
ASSET_NAME="Photon-ESP32-C6-KISS-TCP"

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

build_flags="${PLATFORMIO_BUILD_FLAGS:-}"
build_flags="${build_flags} -DFIRMWARE_BUILD_DATE='\"${FIRMWARE_BUILD_DATE}\"'"
build_flags="${build_flags} -DFIRMWARE_VERSION='\"${PHOTON_FIRMWARE_VERSION}\"'"

if [ -n "${EXTRA_BUILD_FLAGS:-}" ]; then
  build_flags="${build_flags} ${EXTRA_BUILD_FLAGS}"
fi

echo "Building ${ENV_NAME} -> ${ASSET_NAME}-${FIRMWARE_VERSION} (internal ${PHOTON_FIRMWARE_VERSION})"

rm -rf ".pio/build/${ENV_NAME}"

PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -e "${ENV_NAME}"
PLATFORMIO_BUILD_FLAGS="${build_flags}" pio run -t mergebin -e "${ENV_NAME}"

cp ".pio/build/${ENV_NAME}/firmware.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}.bin"
cp ".pio/build/${ENV_NAME}/firmware-merged.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}-merged.bin"

if [ -f ".pio/build/${ENV_NAME}/partitions.bin" ]; then
  cp ".pio/build/${ENV_NAME}/partitions.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}-partitions.bin"
fi
if [ -f ".pio/build/${ENV_NAME}/bootloader.bin" ]; then
  cp ".pio/build/${ENV_NAME}/bootloader.bin" "${OUTPUT_DIR}/${ASSET_NAME}-${FIRMWARE_VERSION}-bootloader.bin"
fi

echo "Built files:"
find "${OUTPUT_DIR}" -maxdepth 1 -type f | sort
